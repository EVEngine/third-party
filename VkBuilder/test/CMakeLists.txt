


if (WIN32) 
    set(GLFW_FOLDER "win32")
    set(SYS_LIB "")
endif()
if (APPLE)
    set(GLFW_FOLDER "macOS")
    set(SYS_LIB dl)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(GLFW_FOLDER "linux")
    set(SYS_LIB X11 dl)
    set(LINK_OPT -pthread)
endif()


add_spirv_shader(vertex ${CMAKE_CURRENT_SOURCE_DIR}/shader/vert.glsl vert.spv)
add_spirv_shader(fragment ${CMAKE_CURRENT_SOURCE_DIR}/shader/frag.glsl frag.spv)

add_custom_target(shaders ALL DEPENDS vert.spv frag.spv)

## path configuration
include_directories(include ${Vulkan_INCLUDE_DIRS})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/${GLFW_FOLDER})

file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(vbktest ${source_files})
target_link_libraries(vbktest ${Vulkan_LIBRARY} glfw3 ${SYS_LIB})
target_link_options(vbktest PRIVATE ${LINK_OPT})