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

# AppleClang / modern macOS SDK: keep vendored deps building without treating
# newer diagnostics as hard errors (especially assimp memcpy warnings).
if(APPLE)
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:-Wno-deprecated-non-prototype>
        $<$<COMPILE_LANGUAGE:C,OBJC>:-Wno-error=declaration-after-statement>
    )
endif()

# Android NDK: shared SDL2 for SDLActivity; OpenSL ES backend for OpenAL Soft.
if(ANDROID OR (CMAKE_SYSTEM_NAME STREQUAL "Android"))
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
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
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(VIDEO_COCOA OFF CACHE BOOL "" FORCE)
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
    add_compile_definitions(IOS=1)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:-Wno-deprecated-non-prototype>
        $<$<COMPILE_LANGUAGE:C,OBJC>:-Wno-error=declaration-after-statement>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-declaration-after-statement>
    )
endif()

# for openal
set(ALSOFT_EXAMPLES OFF CACHE BOOL "Build OpenAL Soft examples" FORCE)
add_subdirectory(openal-soft)

# for freetype
add_subdirectory(freetype)

# for SDL2, googletest and squirrel
add_subdirectory(SDL2)
if(NOT (CMAKE_SYSTEM_NAME STREQUAL "iOS" OR IOS OR ANDROID OR (CMAKE_SYSTEM_NAME STREQUAL "Android")))
    add_subdirectory(googletest)
endif()
add_subdirectory(squirrel)

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
set(ENABLE_DATA OFF CACHE BOOL "" FORCE)
set(ENABLE_DATA_SQLITE OFF CACHE BOOL "" FORCE)
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
add_subdirectory(poco)

# for zlib
add_subdirectory(zlib)

# install glm
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install VkBuilder
file(GLOB_RECURSE vkbuilder_src ${CMAKE_CURRENT_SOURCE_DIR}/VkBuilder/include/*)
install(FILES ${vkbuilder_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install Vulkan Memory Allocator and Vulkan Headers
file(GLOB_RECURSE vma_src ${CMAKE_CURRENT_SOURCE_DIR}/vma/*)
file(GLOB_RECURSE vulkan_headers_src ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*)
file(GLOB_RECURSE vulkan_video_src ${CMAKE_CURRENT_SOURCE_DIR}/vk_video/*)

install(FILES ${vma_src} ${vulkan_headers_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vulkan/ COMPONENT Development)
install(FILES ${vulkan_video_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vk_video/ COMPONENT Development)

# install zeroerr
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/zeroerr.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install header-only
file(GLOB_RECURSE headers_src ${CMAKE_CURRENT_SOURCE_DIR}/header-only/*)
install(FILES ${headers_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install utf8
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/utf8/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# for Box2D
file(GLOB_RECURSE box2d_header ${CMAKE_CURRENT_SOURCE_DIR}/Box2D/**/*.h ${CMAKE_CURRENT_SOURCE_DIR}/Box2D/*.h )
file(GLOB_RECURSE box2d_src ${CMAKE_CURRENT_SOURCE_DIR}/Box2D/**/*.cpp)
install(FILES ${box2d_header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Box2D COMPONENT Development)
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

install(TARGETS physfs Box2D xxHash lz4 DESTINATION ${CMAKE_INSTALL_LIBDIR}/ COMPONENT Development)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zlib/ ${CMAKE_CURRENT_BINARY_DIR}/zlib/)
# for medialoader
add_subdirectory(medialoader)