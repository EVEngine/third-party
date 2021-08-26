cmake_minimum_required(VERSION 3.8)

project(EVEngineDeps)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/binary)
# set(CMAKE_CXX_FLAGS "-fPIC -fcommon")
# set(SDL_STATIC_PIC ON)

add_subdirectory(SDL2)
# add_subdirectory(squall)
add_subdirectory(squirrel)


# install squall
file(GLOB_RECURSE squall_src ${CMAKE_CURRENT_SOURCE_DIR}/squall/squall/*)
install(FILES ${squall_src} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development)
  
# install glm
file(GLOB_RECURSE glm_src ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ COMPONENT Development)

