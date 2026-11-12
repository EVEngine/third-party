# Consume the owning VKBuilder sources directly; never generate or rewrite headers.
if(EVENGINE_THIRD_PARTY_BINARY_DIR)
    set(EVENGINE_VKBUILDER_INCLUDE_DIR "${EVENGINE_THIRD_PARTY_BINARY_DIR}/include")
else()
    set(EVENGINE_VKBUILDER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/VKBuilder/include")
endif()
if(NOT EXISTS "${EVENGINE_VKBUILDER_INCLUDE_DIR}/vkbuilder.hpp")
    message(FATAL_ERROR "Canonical VKBuilder header missing from the selected dependency install")
endif()
