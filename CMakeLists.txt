cmake_minimum_required(VERSION 3.16)

project(EVEngineDeps)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../third-party-binary)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(BUILD_SHARED_LIBS OFF)
set(LIBTYPE STATIC)

# for openal
set(ALSOFT_EXAMPLES OFF)
add_subdirectory(openal-soft)

# for freetype
add_subdirectory(freetype)

# for SDL2, googletest and squirrel
add_subdirectory(SDL2)
add_subdirectory(googletest)
add_subdirectory(squirrel)

# filesystem watcher already has in poco DirectoryWatcher
# add_subdirectory(efsw)

# for simplesquirrel
set(SQUIRREL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/squirrel/include)
set(SQUIRREL_LIBRARIES squirrel_static)
set(SQSTDLIB_LIBRARIESRARIES sqstdlib_static)
add_subdirectory(simplesquirrel)

# for poco
set(POCO_STATIC ON)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(poco)

# for zlib
add_subdirectory(zlib)

# for medialoader
add_subdirectory(medialoader)


# install glm
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install VkBuilder
file(GLOB_RECURSE vkbuilder_src ${CMAKE_CURRENT_SOURCE_DIR}/VkBuilder/include/*)
install(FILES ${vkbuilder_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

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
