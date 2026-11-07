cmake_minimum_required(VERSION 3.21)
project(EveReferenceMath VERSION 0.1.0 LANGUAGES CXX)
if(WIN32 AND NOT MSVC AND NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    message(FATAL_ERROR "Windows reference math requires an MSVC-compatible C++ driver in a Developer environment")
endif()
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(cmake/VerifyVendor.cmake)
eve_reference_math_verify_vendor("${CMAKE_CURRENT_SOURCE_DIR}")
option(EVE_REFERENCE_MATH_BUILD_TESTS "Build standalone numerical API contracts" OFF)
set(_v8 "${CMAKE_CURRENT_SOURCE_DIR}/vendor/v8")
set(_core "${CMAKE_CURRENT_SOURCE_DIR}/vendor/core-math")
set(_sources src/reference_math.cpp "${_v8}/src/base/ieee754.cc")
if(WIN32)
    find_program(EVE_REFERENCE_MATH_CLANG_CL NAMES clang-cl
        DOC "Clang C compiler used only for the canonical CORE-MATH source")
    if(NOT EVE_REFERENCE_MATH_CLANG_CL OR NOT EXISTS "${EVE_REFERENCE_MATH_CLANG_CL}")
        message(FATAL_ERROR "Reference math source builds on Windows require clang-cl; prebuilt consumers do not")
    endif()
    # Processor metadata may describe the Windows host rather than the C++ target.
    # Probe the actual compiler, so the separately compiled C object has the same ABI.
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles([=[
#if !defined(_M_ARM64) || defined(_M_ARM64EC)
#error Expected a native ARM64 compiler target
#endif
int main() { return 0; }
]=] _eve_reference_math_compiler_arm64)
    check_cxx_source_compiles([=[
#if !defined(_M_X64) || defined(_M_ARM64EC)
#error Expected a native x64 compiler target
#endif
int main() { return 0; }
]=] _eve_reference_math_compiler_x64)
    if(_eve_reference_math_compiler_arm64 AND NOT _eve_reference_math_compiler_x64)
        set(_core_target "--target=aarch64-pc-windows-msvc")
    elseif(_eve_reference_math_compiler_x64 AND NOT _eve_reference_math_compiler_arm64)
        set(_core_target "--target=x86_64-pc-windows-msvc")
    else()
        message(FATAL_ERROR "Reference math Windows source build requires an x64 or ARM64 C++ compiler target")
    endif()
    if(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY AND
       NOT CMAKE_MSVC_RUNTIME_LIBRARY STREQUAL "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        message(FATAL_ERROR "Reference math Windows builds require the aggregate DLL CRT policy")
    endif()
    set(_core_obj "${CMAKE_CURRENT_BINARY_DIR}/core-math/$<CONFIG>/pow.obj")
    add_custom_command(OUTPUT "${_core_obj}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/core-math/$<CONFIG>"
        COMMAND "${EVE_REFERENCE_MATH_CLANG_CL}" "${_core_target}" /nologo /TC /c /O2 /fp:strict
            "$<IF:$<CONFIG:Debug>,/MDd,/MD>" "${_core}/pow.c" "/Fo${_core_obj}"
        DEPENDS "${_core}/pow.c" "${_core}/pow.h" "${_core}/dint.h" "${_core}/qint.h"
        VERBATIM)
    set_source_files_properties("${_core_obj}" PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
    list(APPEND _sources "${_core_obj}")
else()
    enable_language(C)
    list(APPEND _sources "${_core}/pow.c")
endif()
add_library(eve_reference_math STATIC ${_sources})
add_library(EveReferenceMath::eve_reference_math ALIAS eve_reference_math)
target_compile_features(eve_reference_math PRIVATE cxx_std_20)
target_include_directories(eve_reference_math
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE "${_v8}")
target_compile_definitions(eve_reference_math PRIVATE NDEBUG)
set_target_properties(eve_reference_math PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(WIN32)
    target_compile_options(eve_reference_math PRIVATE /utf-8 /fp:strict /Zc:__cplusplus)
else()
    target_compile_options(eve_reference_math PRIVATE -O2 -ffp-contract=off -frounding-math -fno-fast-math)
    target_link_libraries(eve_reference_math PUBLIC m)
endif()
# Strict warnings cover owned adapters/contracts; canonical vendor sources stay unchanged.
if(WIN32)
    set_property(SOURCE src/reference_math.cpp tests/contracts.cpp APPEND PROPERTY COMPILE_OPTIONS /W4 /WX)
else()
    set_property(SOURCE src/reference_math.cpp tests/contracts.cpp APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror)
endif()
install(TARGETS eve_reference_math EXPORT EveReferenceMathTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES include/eve/reference_math.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/eve")
install(EXPORT EveReferenceMathTargets NAMESPACE EveReferenceMath::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EveReferenceMath")
configure_package_config_file(EveReferenceMathConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/EveReferenceMathConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EveReferenceMath")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/EveReferenceMathConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/EveReferenceMathConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EveReferenceMathConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EveReferenceMath")
install(FILES README.md UPSTREAM.json LICENSE LICENSE-OPENSOURCE LICENSE-COMMERCIAL
    DESTINATION "${CMAKE_INSTALL_DATADIR}/eve/licenses/reference-math")
install(DIRECTORY vendor/
    DESTINATION "${CMAKE_INSTALL_DATADIR}/eve/licenses/reference-math/vendor")
if(EVE_REFERENCE_MATH_BUILD_TESTS)
    enable_testing()
    add_executable(eve_reference_math_contracts tests/contracts.cpp)
    target_compile_features(eve_reference_math_contracts PRIVATE cxx_std_20)
    target_link_libraries(eve_reference_math_contracts PRIVATE EveReferenceMath::eve_reference_math)
    add_test(NAME reference-math.contracts COMMAND eve_reference_math_contracts)
endif()
