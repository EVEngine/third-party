cmake_minimum_required(VERSION 3.16)

project(EVEngineDeps)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/binary)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_MESSAGE LAZY)

add_subdirectory(SDL2)
add_subdirectory(googletest)
add_subdirectory(squirrel)

# for simplesquirrel 
set(SQUIRREL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/squirrel/include)
set(SQUIRREL_LIBRARIES squirrel_static)
set(SQSTDLIB_LIBRARIESRARIES sqstdlib_static)
add_subdirectory(simplesquirrel)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(poco)


# install squall
file(GLOB_RECURSE squall_src ${CMAKE_CURRENT_SOURCE_DIR}/squall/squall/*)
install(FILES ${squall_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development)
  
# install glm
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

# install VkBuilder
file(GLOB_RECURSE vkbuilder_src ${CMAKE_CURRENT_SOURCE_DIR}/VkBuilder/include/*)
install(FILES ${vkbuilder_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)


# install header-only
file(GLOB_RECURSE headers_src ${CMAKE_CURRENT_SOURCE_DIR}/header-only/*)
install(FILES ${headers_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)