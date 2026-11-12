include_guard(GLOBAL)
include(ExternalProject)

# OpenSSL owns its build system. Never pick up a host SSL installation when
# producing a cross-compiled or redistributable dependency prefix.
function(eve_add_openssl)
    if(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
        message(FATAL_ERROR "Set CMAKE_BUILD_TYPE explicitly for the dependency ABI")
    endif()
    if(CMAKE_CONFIGURATION_TYPES)
        # One aggregate install prefix has one CRT ABI, including when the
        # engine's source-build path uses the Visual Studio generator.
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "Dependency configuration" FORCE)
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" PARENT_SCOPE)
    endif()
    if(CMAKE_HOST_WIN32 AND ANDROID AND NOT PERL_EXECUTABLE)
        # Android uses OpenSSL's Unix makefiles, so its Perl and Make must
        # belong to one complete POSIX environment, not Git's partial Perl.
        find_program(PERL_EXECUTABLE NAMES perl
            PATHS "$ENV{EVE_OPENSSL_MSYS2_ROOT}/usr/bin" "C:/msys64/usr/bin"
            NO_DEFAULT_PATH)
    endif()
    if(MSVC AND NOT PERL_EXECUTABLE)
        # Git Bash prepends its MSYS Perl even when GITHUB_PATH contains
        # Strawberry. Prefer the native installation before general discovery;
        # an explicit caller-provided PERL_EXECUTABLE always takes precedence.
        find_program(PERL_EXECUTABLE NAMES perl
            PATHS "C:/Strawberry/perl/bin" "$ENV{ProgramFiles}/Strawberry/perl/bin"
            NO_DEFAULT_PATH)
    endif()
    find_package(Perl REQUIRED)
    if(MSVC)
        execute_process(COMMAND "${PERL_EXECUTABLE}" -e "print $^O"
            OUTPUT_VARIABLE _perl_platform RESULT_VARIABLE _perl_result)
        if(NOT _perl_result EQUAL 0 OR NOT _perl_platform STREQUAL "MSWin32")
            message(FATAL_ERROR "MSVC OpenSSL requires native Windows Perl (e.g. Strawberry), not MSYS/Cygwin Perl")
        endif()
    endif()
    set(_android_ndk "${CMAKE_ANDROID_NDK}")
    set(_openssl_cygpath "")
    if(CMAKE_HOST_WIN32 AND ANDROID)
        execute_process(COMMAND "${PERL_EXECUTABLE}" -MIPC::Cmd -e "print $^O"
            OUTPUT_VARIABLE _perl_platform RESULT_VARIABLE _perl_result
            ERROR_VARIABLE _perl_error)
        if(NOT _perl_result EQUAL 0)
            message(FATAL_ERROR "Android OpenSSL needs a complete Perl with IPC::Cmd: ${_perl_error}")
        endif()
        if(_perl_platform MATCHES "^(msys|cygwin)$")
            get_filename_component(_perl_bin "${PERL_EXECUTABLE}" DIRECTORY)
            find_program(_openssl_cygpath_program NAMES cygpath PATHS "${_perl_bin}" NO_DEFAULT_PATH REQUIRED)
            find_program(_openssl_posix_make NAMES make PATHS "${_perl_bin}" NO_DEFAULT_PATH REQUIRED)
            set(_openssl_cygpath "${_openssl_cygpath_program}")
            execute_process(COMMAND "${_openssl_cygpath}" -u "${_android_ndk}"
                OUTPUT_VARIABLE _android_ndk OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE _path_result)
            if(NOT _path_result EQUAL 0)
                message(FATAL_ERROR "Could not convert the Android NDK path for the selected Perl")
            endif()
        endif()
    endif()
    set(_source "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../openssl")
    if(NOT EXISTS "${_source}/Configure")
        message(FATAL_ERROR "OpenSSL sources missing: run git submodule update --init --recursive")
    endif()
    foreach(_part MAJOR MINOR PATCH)
        file(STRINGS "${_source}/VERSION.dat" _line REGEX "^${_part}=")
        string(REGEX REPLACE "^${_part}=" "" _${_part} "${_line}")
    endforeach()
    set(EVE_OPENSSL_VERSION "${_MAJOR}.${_MINOR}.${_PATCH}" PARENT_SCOPE)
    set(_prefix "${CMAKE_CURRENT_BINARY_DIR}/openssl-install")
    set(_configure_prefix "${_prefix}")
    if(_openssl_cygpath)
        execute_process(COMMAND "${_openssl_cygpath}" -u "${_prefix}"
            OUTPUT_VARIABLE _configure_prefix OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE _path_result)
        if(NOT _path_result EQUAL 0)
            message(FATAL_ERROR "Could not convert the OpenSSL install path for the selected Perl")
        endif()
    endif()
    set(_options no-shared no-tests no-apps no-module no-legacy no-asm
        "--prefix=${_configure_prefix}" --libdir=lib)
    set(_env)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _arch)
    if(MSVC)
        find_program(_make nmake REQUIRED)
        if(_arch MATCHES "^(arm64|aarch64)$")
            set(_target VC-WIN64-ARM)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_target VC-WIN64A)
        else()
            set(_target VC-WIN32)
        endif()
        set(_crypto "${_prefix}/lib/libcrypto.lib")
        set(_ssl "${_prefix}/lib/libssl.lib")
        set(_system_libs ws2_32 crypt32 advapi32 user32 gdi32)
        # OpenSSL's no-shared default is /MT. Explicit user flags follow its
        # library flags, selecting the same DLL CRT as the aggregate.
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            list(APPEND _options /MDd)
        else()
            list(APPEND _options /MD)
        endif()
    else()
        if(_openssl_posix_make)
            set(_make "${_openssl_posix_make}")
        else()
            find_program(_make NAMES gmake make REQUIRED)
        endif()
        set(_crypto "${_prefix}/lib/libcrypto.a")
        set(_ssl "${_prefix}/lib/libssl.a")
        set(_system_libs "${CMAKE_DL_LIBS}")
        if(ANDROID)
            if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
                set(_target android-arm64)
            elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
                set(_target android-arm)
            elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
                set(_target android-x86)
            elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
                set(_target android-x86_64)
            else()
                message(FATAL_ERROR "Unsupported OpenSSL Android ABI: ${CMAKE_ANDROID_ARCH_ABI}")
            endif()
            get_filename_component(_tools "${CMAKE_C_COMPILER}" DIRECTORY)
            if(CMAKE_HOST_WIN32)
                if(_openssl_cygpath)
                    set(_path "${_tools};${_perl_bin};$ENV{PATH}")
                else()
                    set(_path "${_tools};$ENV{PATH}")
                endif()
            else()
                set(_path "${_tools}:$ENV{PATH}")
            endif()
            # ExternalProject expands CMake lists before constructing commands.
            # Encode literal semicolons until its final command generation step.
            string(REPLACE ";" "<EVE_OPENSSL_SEMICOLON>" _path "${_path}")
            list(APPEND _env "ANDROID_NDK_ROOT=${_android_ndk}" "PATH=${_path}")
            # The NDK's legacy toolchain sets CMAKE_SYSTEM_VERSION to 1;
            # ANDROID_PLATFORM_LEVEL is its resolved native API (including
            # NDK minimum-level adjustments). CMake's own Android toolchain
            # instead records that API in CMAKE_SYSTEM_VERSION.
            if(DEFINED ANDROID_PLATFORM_LEVEL)
                set(_android_api "${ANDROID_PLATFORM_LEVEL}")
            else()
                set(_android_api "${CMAKE_SYSTEM_VERSION}")
            endif()
            if(NOT _android_api MATCHES "^[0-9]+$" OR _android_api LESS 16)
                message(FATAL_ERROR "Invalid OpenSSL Android native API: ${_android_api}")
            endif()
            list(APPEND _options "-D__ANDROID_API__=${_android_api}")
        elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
            string(TOLOWER "${CMAKE_OSX_SYSROOT}" _sdk)
            if(_sdk MATCHES "simulator")
                if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
                    set(_target iossimulator-arm64-xcrun)
                elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
                    set(_target iossimulator-x86_64-xcrun)
                else()
                    message(FATAL_ERROR "OpenSSL needs one iOS simulator architecture per build")
                endif()
                list(APPEND _options "-mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
            else()
                set(_target ios64-xcrun)
                list(APPEND _options "-mios-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
            endif()
        elseif(APPLE)
            if(CMAKE_OSX_ARCHITECTURES)
                set(_arch "${CMAKE_OSX_ARCHITECTURES}")
            endif()
            if(_arch STREQUAL "arm64")
                set(_target darwin64-arm64-cc)
            elseif(_arch STREQUAL "x86_64")
                set(_target darwin64-x86_64-cc)
            else()
                message(FATAL_ERROR "OpenSSL needs one Apple architecture per build")
            endif()
            list(APPEND _options "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            if(_arch MATCHES "^(arm64|aarch64)$")
                set(_target linux-aarch64)
            elseif(_arch MATCHES "^(x86_64|amd64)$")
                set(_target linux-x86_64)
            else()
                message(FATAL_ERROR "Unsupported OpenSSL Linux architecture: ${_arch}")
            endif()
            list(APPEND _env "CC=${CMAKE_C_COMPILER}" "AR=${CMAKE_AR}" "RANLIB=${CMAKE_RANLIB}")
        else()
            message(FATAL_ERROR "Unsupported bundled OpenSSL target: ${CMAKE_SYSTEM_NAME}")
        endif()
        list(APPEND _options -fPIC)
        find_package(Threads REQUIRED)
        list(APPEND _system_libs Threads::Threads)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND _options --debug)
    else()
        list(APPEND _options --release)
    endif()
    ExternalProject_Add(eve_openssl
        LIST_SEPARATOR "<EVE_OPENSSL_SEMICOLON>"
        SOURCE_DIR "${_source}" DOWNLOAD_COMMAND "" UPDATE_COMMAND ""
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${_env}
            "${PERL_EXECUTABLE}" "${_source}/Configure" ${_target} ${_options}
        BUILD_COMMAND ${CMAKE_COMMAND} -E env ${_env} "${_make}" build_libs
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env ${_env} "${_make}" install_dev
        BUILD_BYPRODUCTS "${_crypto}" "${_ssl}")
    file(MAKE_DIRECTORY "${_prefix}/include")
    add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${_crypto}"
        INTERFACE_INCLUDE_DIRECTORIES "${_prefix}/include"
        INTERFACE_LINK_LIBRARIES "${_system_libs}")
    add_dependencies(OpenSSL::Crypto eve_openssl)
    add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${_ssl}"
        INTERFACE_INCLUDE_DIRECTORIES "${_prefix}/include"
        INTERFACE_LINK_LIBRARIES OpenSSL::Crypto)
    add_dependencies(OpenSSL::SSL eve_openssl)
    # The normal installed FindOpenSSL contract is used by downstream packages.
    # During this build the libraries/headers are installed by ExternalProject.
    set(EVE_OPENSSL_PREFIX "${_prefix}" PARENT_SCOPE)
    install(DIRECTORY "${_prefix}/include/" DESTINATION include)
    install(FILES "${_crypto}" "${_ssl}" DESTINATION lib)
    install(FILES "${_source}/LICENSE.txt" DESTINATION share/licenses/OpenSSL)
endfunction()
