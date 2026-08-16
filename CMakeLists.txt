cmake_minimum_required(VERSION 3.21)

project(EVEngineDeps)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
set(CMAKE_INSTALL_MESSAGE LAZY CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LIBTYPE STATIC CACHE BOOL "" FORCE)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Vendored assimp pulls stb_image.h; GCC/Clang -Wsign-compare (and similar) become
# hard failures when ASSIMP_WARNINGS_AS_ERRORS is left ON (default).
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

# Chinese Windows (code page 936): UTF-8 sources (e.g. FreeType) otherwise trigger C4819.
# Do not add charset flags globally: /utf-8 is shorthand for
# /source-charset:utf-8 /execution-charset:utf-8, and MSVC D8016 if both forms
# appear. Assimp already passes /source-charset:utf-8; googletest already
# passes -utf-8. Per-target /utf-8 is applied after all subdirectories.
function(eve_target_has_msvc_charset tgt dir out)
    set(_found FALSE)
    foreach(_prop COMPILE_OPTIONS INTERFACE_COMPILE_OPTIONS COMPILE_FLAGS)
        get_target_property(_val ${tgt} ${_prop})
        if(_val AND ("${_val}" MATCHES "utf-8" OR "${_val}" MATCHES "source-charset"
                     OR "${_val}" MATCHES "execution-charset"))
            set(_found TRUE)
        endif()
    endforeach()
    get_directory_property(_dir_opts DIRECTORY "${dir}" COMPILE_OPTIONS)
    if(_dir_opts AND ("${_dir_opts}" MATCHES "utf-8" OR "${_dir_opts}" MATCHES "source-charset"
                      OR "${_dir_opts}" MATCHES "execution-charset"))
        set(_found TRUE)
    endif()
    set(${out} ${_found} PARENT_SCOPE)
endfunction()

function(eve_msvc_utf8_if_needed dir)
    get_property(_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(tgt IN LISTS _targets)
        if(NOT TARGET "${tgt}")
            continue()
        endif()
        get_target_property(_imported ${tgt} IMPORTED)
        if(_imported)
            continue()
        endif()
        get_target_property(_type ${tgt} TYPE)
        if(_type STREQUAL "INTERFACE_LIBRARY" OR _type STREQUAL "UTILITY" OR _type STREQUAL "ALIAS")
            continue()
        endif()
        eve_target_has_msvc_charset(${tgt} "${dir}" _has)
        if(_has)
            continue()
        endif()
        target_compile_options(${tgt} PRIVATE /utf-8)
    endforeach()
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(sub IN LISTS _subdirs)
        eve_msvc_utf8_if_needed("${sub}")
    endforeach()
endfunction()

# AppleClang / modern macOS SDK: keep vendored deps building without treating
# newer diagnostics as hard errors (especially assimp memcpy warnings).
if(APPLE)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:-Wno-deprecated-non-prototype>
        $<$<COMPILE_LANGUAGE:C,OBJC>:-Wno-error=declaration-after-statement>
    )
endif()

# Android NDK: shared SDL2 for SDLActivity; OpenSL ES backend for OpenAL Soft.
if(ANDROID OR (CMAKE_SYSTEM_NAME STREQUAL "Android"))
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_OPENSL ON CACHE BOOL "" FORCE)
    set(ALSOFT_BACKEND_WAVE OFF CACHE BOOL "" FORCE)
    set(ALSOFT_REQUIRE_OPENSL ON CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
    # Keep other deps static.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    # mpg123: skip CLI tools under NDK; FPU is forced in mpg123 CMakeLists for Android.
    set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(BUILD_LIBOUT123 OFF CACHE BOOL "" FORCE)
    # SDL OpenSL / NDK headers mix declarations after statements; SDL enables -Werror=DAS.
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-error=declaration-after-statement>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-declaration-after-statement>
    )
endif()

# iOS / iPadOS: static SDL UIKit + CoreAudio OpenAL Soft; no host Cocoa path.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR IOS)
    set(IOS TRUE CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(VIDEO_COCOA OFF CACHE BOOL "" FORCE)
    set(VIDEO_OPENGL OFF CACHE BOOL "" FORCE)
    set(VIDEO_OPENGLES ON CACHE BOOL "" FORCE)
    set(VIDEO_VULKAN ON CACHE BOOL "" FORCE)
    set(VIDEO_METAL ON CACHE BOOL "" FORCE)
    set(RENDER_METAL ON CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(BUILD_LIBOUT123 OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    # Deps are libraries only; do not create iOS app bundles for CLI tools.
    set(CMAKE_MACOSX_BUNDLE OFF)
    set(DISABLE_DYNAMIC ON CACHE BOOL "" FORCE)
    # Do NOT add_compile_definitions(IOS=1): Poco uses a typedef named IOS.
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:-Wno-deprecated-non-prototype>
        $<$<COMPILE_LANGUAGE:C,OBJC>:-Wno-error=declaration-after-statement>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-declaration-after-statement>
    )
    message(STATUS "iOS deps: UIKit SDL (VIDEO_COCOA=OFF VIDEO_OPENGL=OFF)")
endif()

# WebGPU browser (Emscripten): trim the dependency set hard. SDL2 and zlib come
# from Emscripten ports (-sUSE_SDL=2 / -sUSE_ZLIB); Poco / OpenAL / medialoader
# (assimp + mpg123/vorbis/ogg) / freetype are not buildable on WASM and the
# corresponding engine modules (network/database/sound/audio/animation/model3d/
# font/...) are compiled out. Keep only the simple, WASM-friendly libraries.
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EVENGINE_WEBGPU_TRIM TRUE CACHE BOOL "" FORCE)
    message(STATUS "Emscripten deps: trimming third-party (Poco/OpenAL/medialoader/freetype skipped; SDL2+zlib via Emscripten ports)")
    # The engine links with PROXY_TO_PTHREAD (shared memory), which requires
    # every TU — including third-party archives — to be built with atomics and
    # bulk-memory or wasm-ld rejects the object at link time.
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX>:-matomics>
        $<$<COMPILE_LANGUAGE:C,CXX>:-mbulk-memory>
    )
endif()

# for openal
set(ALSOFT_EXAMPLES OFF CACHE BOOL "Build OpenAL Soft examples" FORCE)
if(NOT EVENGINE_WEBGPU_TRIM)
  add_subdirectory(openal-soft)
endif()

# for freetype — keep the static lib free of optional host deps (png/bzip2/harfbuzz).
# Otherwise find_package() may enable them when present on the build machine, and
# every engine link would need those system libraries.
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG TRUE CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 TRUE CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec TRUE CACHE BOOL "" FORCE)
if(NOT EVENGINE_WEBGPU_TRIM)
  add_subdirectory(freetype)
endif()

# for SDL2, googletest and squirrel
# SDL2 vendors .m/.mm sources on Apple which need OBJC/OBJCXX enabled in the
# parent project before its targets are created (a later nested project() would
# otherwise reset them, leaving CMAKE_OBJC_COMPILE_OBJECT unset at generate).
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()
# On Emscripten SDL2 is provided by the Emscripten port (-sUSE_SDL=2); skip the
# vendored tree entirely (it drags in host-platform backends that do not build).
if(NOT EVENGINE_WEBGPU_TRIM)
  add_subdirectory(SDL2)
endif()
if(NOT (CMAKE_SYSTEM_NAME STREQUAL "iOS" OR IOS OR ANDROID OR (CMAKE_SYSTEM_NAME STREQUAL "Android") OR EVENGINE_WEBGPU_TRIM))
    add_subdirectory(googletest)
endif()
# Emscripten: WASM shared libraries are poorly supported (the dynamic sq
# interpreter fails to link its .so deps). Build squirrel static-only and skip
# the sq / sq_static CLI tools from the ALL graph — the engine links the static
# archives, not the interpreter.
if(EVENGINE_WEBGPU_TRIM)
    set(DISABLE_DYNAMIC ON CACHE BOOL "Squirrel static-only on Emscripten" FORCE)
endif()
add_subdirectory(squirrel)
if(EVENGINE_WEBGPU_TRIM)
    # sq_static links fine with the WASM C++ runtime; the dynamic sq target is
    # gone (DISABLE_DYNAMIC). Keep sq_static buildable so its install rule
    # succeeds — the engine never links the interpreter binary itself.
    if(TARGET sq_static)
        set_property(TARGET sq_static APPEND PROPERTY LINK_LIBRARIES "c++;c++abi")
    endif()
endif()

# filesystem watcher already has in poco DirectoryWatcher
# add_subdirectory(efsw)

# for simplesquirrel
set(SQUIRREL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/squirrel/include CACHE STRING "" FORCE)
set(SQUIRREL_LIBRARIES squirrel_static CACHE STRING "" FORCE)
set(SQSTDLIB_LIBRARIESRARIES sqstdlib_static CACHE STRING "" FORCE)
add_subdirectory(simplesquirrel)

# for poco
set(POCO_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# EVEngine only links Foundation/Net/JSON/XML; disable the rest so AppleClang
# does not need to build broken optional components (Data/BLOB, etc.).
set(ENABLE_DATA ON CACHE BOOL "" FORCE)
set(ENABLE_DATA_SQLITE ON CACHE BOOL "" FORCE)
set(ENABLE_DATA_MYSQL OFF CACHE BOOL "" FORCE)
set(ENABLE_DATA_POSTGRESQL OFF CACHE BOOL "" FORCE)
set(ENABLE_DATA_ODBC OFF CACHE BOOL "" FORCE)
set(ENABLE_MONGODB OFF CACHE BOOL "" FORCE)
set(ENABLE_REDIS OFF CACHE BOOL "" FORCE)
set(ENABLE_JWT OFF CACHE BOOL "" FORCE)
set(ENABLE_NETSSL OFF CACHE BOOL "" FORCE)
set(ENABLE_CRYPTO OFF CACHE BOOL "" FORCE)
set(ENABLE_ZIP OFF CACHE BOOL "" FORCE)
set(ENABLE_PAGECOMPILER OFF CACHE BOOL "" FORCE)
set(ENABLE_PAGECOMPILER_FILE2PAGE OFF CACHE BOOL "" FORCE)
set(ENABLE_ACTIVERECORD OFF CACHE BOOL "" FORCE)
set(ENABLE_ACTIVERECORD_COMPILER OFF CACHE BOOL "" FORCE)
set(ENABLE_UTIL OFF CACHE BOOL "" FORCE)
if(NOT EVENGINE_WEBGPU_TRIM)
  add_subdirectory(poco)
endif()

# for zlib
# On Emscripten zlib comes from the port (-sUSE_ZLIB); skip the vendored tree.
if(NOT EVENGINE_WEBGPU_TRIM)
  add_subdirectory(zlib)
endif()

# for Box3D (Erin Catto) — 3D rigid bodies.
set(BOX3D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX3D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX3D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX3D_DOCS OFF CACHE BOOL "" FORCE)
set(BOX3D_VALIDATE OFF CACHE BOOL "" FORCE)
add_subdirectory(box3d)

# install glm
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install Vulkan Memory Allocator and Vulkan Headers
file(GLOB_RECURSE vma_src ${CMAKE_CURRENT_SOURCE_DIR}/vma/*)
file(GLOB_RECURSE vulkan_headers_src ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*)
file(GLOB_RECURSE vulkan_video_src ${CMAKE_CURRENT_SOURCE_DIR}/vk_video/*)

install(FILES ${vma_src} ${vulkan_headers_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vulkan/ COMPONENT Development)
install(FILES ${vulkan_video_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vk_video/ COMPONENT Development)

# install header-only
file(GLOB_RECURSE headers_src ${CMAKE_CURRENT_SOURCE_DIR}/header-only/*)
install(FILES ${headers_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install Dungeon Template Library (header-only)
file(GLOB_RECURSE dtl_src ${CMAKE_CURRENT_SOURCE_DIR}/DungeonTemplateLibrary/include/*)
install(FILES ${dtl_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install utf8
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/utf8/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# for Box2D
# Preserve Collision/Common/Dynamics layout so <Box2D/Common/b2Settings.h> resolves.
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Box2D/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Box2D
        COMPONENT Development
        FILES_MATCHING PATTERN "*.h")
file(GLOB_RECURSE box2d_src ${CMAKE_CURRENT_SOURCE_DIR}/Box2D/**/*.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
add_library(Box2D ${box2d_src})

# for Physfs
file(GLOB_RECURSE physfs_header ${CMAKE_CURRENT_SOURCE_DIR}/physfs/*.h )
file(GLOB_RECURSE physfs_src ${CMAKE_CURRENT_SOURCE_DIR}/physfs/*.c ${CMAKE_CURRENT_SOURCE_DIR}/physfs/*.cpp)
install(FILES ${physfs_header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/physfs COMPONENT Development)
if(APPLE)
set(physfs_src ${physfs_src} ${CMAKE_CURRENT_SOURCE_DIR}/physfs/physfs_platform_apple.m)
endif()
add_library(physfs ${physfs_src})

# for lz4
file(GLOB_RECURSE lz4_header ${CMAKE_CURRENT_SOURCE_DIR}/lz4/*.h )
file(GLOB_RECURSE lz4_src ${CMAKE_CURRENT_SOURCE_DIR}/lz4/*.c ${CMAKE_CURRENT_SOURCE_DIR}/lz4/*.cpp)
install(FILES ${lz4_header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lz4 COMPONENT Development)
add_library(lz4 ${lz4_src})

# for xxhash
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/xxHash/xxhash.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xxHash COMPONENT Development)
add_library(xxHash ${CMAKE_CURRENT_SOURCE_DIR}/xxHash/xxhash.c)

# for Dear ImGui (declarative UI backend; headers + SDL/Vulkan impl)
set(_eve_imgui_src
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_sdl.cpp
)
if(EVENGINE_WEBGPU_TRIM)
  # WebGPU renderer lives in the engine (ui/imgui/imgui_impl_wgpu.cpp); do not
  # pull the Vulkan impl into the third-party static lib.
else()
  list(APPEND _eve_imgui_src ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp)
endif()
add_library(imgui STATIC ${_eve_imgui_src})
target_include_directories(imgui PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>
  $<INSTALL_INTERFACE:include>
)
# SDL / Vulkan resolved by consumers; keep compile-only here.
target_include_directories(imgui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include)
if(EVENGINE_WEBGPU_TRIM)
  # Emscripten: SDL2 headers come from the port sysroot, not vendored.
  target_include_directories(imgui PRIVATE ${EMSCRIPTEN_SYSROOT}/include)
else()
  find_package(Vulkan QUIET)
  if(Vulkan_FOUND AND Vulkan_INCLUDE_DIRS)
    target_include_directories(imgui PRIVATE ${Vulkan_INCLUDE_DIRS})
  else()
    target_include_directories(imgui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan)
  endif()
endif()
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_sdl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_vulkan.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
)

install(TARGETS physfs Box2D xxHash lz4 imgui DESTINATION ${CMAKE_INSTALL_LIBDIR}/ COMPONENT Development)

if(NOT EVENGINE_WEBGPU_TRIM)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zlib/ ${CMAKE_CURRENT_BINARY_DIR}/zlib/)
  # Assimp (via medialoader) sets ASSIMP_BUILD_ZLIB=OFF to avoid a duplicate zlib target,
  # then runs its own FindZLIB which only searches system paths. Windows CI has no system
  # zlib, so pre-bind the vendored zlibstatic target before configuring medialoader.
  if(TARGET zlibstatic)
      set(ZLIB_FOUND TRUE)
      set(ZLIB_LIBRARY zlibstatic)
      set(ZLIB_LIBRARIES zlibstatic)
      set(ZLIB_INCLUDE_DIR
          "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
          "${CMAKE_CURRENT_BINARY_DIR}/zlib")
      list(PREPEND CMAKE_INCLUDE_PATH
          "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
          "${CMAKE_CURRENT_BINARY_DIR}/zlib")
      message(STATUS "third-party: exposing zlibstatic to Assimp/FindZLIB")
  endif()
  # for medialoader
  add_subdirectory(medialoader)
else()
  # Emscripten: full medialoader is skipped (assimp / sound libs won't build on
  # WASM), but the engine's EVImage module needs the image decoders. Build a
  # minimal medialoader_image from the vendored sources + image-libs
  # (stb / lodepng / ddsparse / tinyexr); no external jpeg/png/tiff/webp.
  file(GLOB_RECURSE _eve_ml_img_src
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/image/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/image-libs/*.cpp")
  add_library(medialoader_image STATIC ${_eve_ml_img_src})
  target_include_directories(medialoader_image PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/image-libs")
  install(TARGETS medialoader_image DESTINATION ${CMAKE_INSTALL_LIBDIR}/ COMPONENT Development)
  # The engine headers still include the header-only assimp interface
  # (<assimp/matrix4x4.h>) and the medialoader public headers; install them so
  # the includes resolve without linking assimp.
  file(GLOB_RECURSE _eve_assimp_hdr
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/model-libs/assimp/include/*.h")
  # config.h.in is a near-complete config.h; configure it (no double precision).
  set(ASSIMP_DOUBLE_PRECISION OFF)
  configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/model-libs/assimp/include/assimp/config.h.in"
      "${CMAKE_CURRENT_BINARY_DIR}/assimp_config.h"
      @ONLY)
  install(FILES ${_eve_assimp_hdr}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/assimp
      COMPONENT Development)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/assimp_config.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/assimp
      RENAME config.h
      COMPONENT Development)
  file(GLOB_RECURSE _eve_medialoader_hdr
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/include/*.h"
      "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/include/*.hpp")
  # install(DIRECTORY) preserves the image/ sound/ ... subdirectory layout.
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/medialoader/include/"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      COMPONENT Development
      FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endif()

if(MSVC)
    eve_msvc_utf8_if_needed("${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Vendored libraries call project(LANGUAGES ...) in their subdirectories, which
# re-initializes the project's language set and can drop OBJC/OBJCXX that SDL2
# (and other Apple libs) need for their .m/.mm sources. Re-enable them at the
# very end so they survive to the generate step.
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()