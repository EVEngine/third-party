cmake_minimum_required(VERSION 3.15)

project(MediaLoader)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(BUILD_SHARED_LIBS OFF)
set(LIBTYPE STATIC)

set(INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)
if (DEP_PATH)
	include_directories(${DEP_PATH}/include)
	set(INSTALL_PATH ${DEP_PATH})
	message(STATUS "Using DEP_PATH: ${DEP_PATH}")
endif()

set(CMAKE_INSTALL_INCLUDEDIR ${INSTALL_PATH}/include)
set(CMAKE_INSTALL_LIBDIR ${INSTALL_PATH}/lib)


include(ExternalProject)
ExternalProject_Add(libogg
	SOURCE_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/sound-libs/libogg/
	BINARY_DIR       ${CMAKE_CURRENT_BINARY_DIR}/libogg-build/
	CMAKE_ARGS       -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
	BUILD_COMMAND    ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libogg-build/ --parallel 8
	BUILD_ALWAYS     1
)

ExternalProject_Add(libvorbis
	SOURCE_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/sound-libs/libvorbis/
	BINARY_DIR       ${CMAKE_CURRENT_BINARY_DIR}/libvorbis-build/
	CMAKE_ARGS       -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
	                 -DOGG_INCLUDE_DIR=${INSTALL_PATH}/include -DOGG_LIBRARY=${INSTALL_PATH}/lib/libogg.a
	BUILD_COMMAND    ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libvorbis-build/ --parallel 8
	BUILD_ALWAYS     1
)

add_dependencies(libvorbis libogg)

ExternalProject_Add(mpg123
	SOURCE_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/sound-libs/mpg123/ports/cmake/
	BINARY_DIR       ${CMAKE_CURRENT_BINARY_DIR}/mpg123-build/
	CMAKE_ARGS       -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
	BUILD_COMMAND    ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/mpg123-build/ --parallel 8
	BUILD_ALWAYS     1
)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}/sound-libs/libmodplug/
	${INSTALL_PATH}
	include image-libs/ sound-libs/ 
	sound-libs/mpg123/src/)

add_subdirectory(sound-libs/libmodplug)
# add_subdirectory(sound-libs/libvorbis)


# set(ASSIMP_BUILD_TESTS OFF CACHE BOOL " " FORCE)
# add_subdirectory(3dmodel-libs/assimp)


file(GLOB image_src ${CMAKE_CURRENT_SOURCE_DIR}/image/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/image-libs/**/*.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/image-libs/**/*.cpp )

add_library(medialoader_image ${image_src})


if(MSVC)
	set(CMAKE_SUPPRESS_REGENERATION TRUE)
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
	if(MEGA_X86)
		# This prevents usage of SSE2.
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /arch:SSE")
	endif()
	message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
	message(STATUS "CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
endif()


file(GLOB sound_src ${CMAKE_CURRENT_SOURCE_DIR}/sound/*.cpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/sound-libs/Wuff/*.c)

add_library(medialoader_sound ${sound_src})
add_dependencies(medialoader_sound libvorbis mpg123)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

install(TARGETS medialoader_image medialoader_sound DESTINATION ${CMAKE_INSTALL_LIBDIR}/ COMPONENT Development)
