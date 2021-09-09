cmake_minimum_required(VERSION 3.15)
project(VkBuilder)

## Version settings
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "1")
SET(VERSION_PATCH "0")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF()

## CPP Compiler 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# VULKAN_SDK is used to hint the vulkan location
option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan in ${Vulkan_INCLUDE_DIR}")

function(add_spirv_shader SHADER_STAGE INPUT_FILE OUTPUT_FILE)
add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND glslc --target-env=vulkan1.2 -fshader-stage=${SHADER_STAGE} ${INPUT_FILE} -o ${OUTPUT_FILE}
        MAIN_DEPENDENCY ${INPUT_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endfunction()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


## path configuration
include_directories(include ${Vulkan_INCLUDE_DIR})
add_subdirectory(test)