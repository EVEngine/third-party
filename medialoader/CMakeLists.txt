cmake_minimum_required(VERSION 3.8)

project(MediaLoader)

set (CMAKE_CXX_STANDARD 11)

add_subdirectory(sound-libs/libvorbis)
add_subdirectory(sound-libs/mpg123/ports/cmake)
add_subdirectory(sound-libs/libmodplug)


include_directories(include image-libs/ sound-libs/ ${CMAKE_CURRENT_BINARY_DIR}/sound-libs/mpg123/ports/cmake/src/ sound-libs/mpg123/src/)

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


file(GLOB sound_src ${CMAKE_CURRENT_SOURCE_DIR}/sound/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/sound-libs/**/*.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/sound-libs/Wuff/*.c)

add_library(medialoader_sound ${sound_src})



install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

install(TARGETS medialoader_image medialoader_sound DESTINATION ${CMAKE_INSTALL_LIBDIR}/ COMPONENT Development)