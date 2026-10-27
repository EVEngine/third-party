# Platform integration for the canonical dependency headers.
set(_vkbuilder_header "${CMAKE_CURRENT_SOURCE_DIR}/third-party/VKBuilder/include/vkbuilder.hpp")
set(EVENGINE_VKBUILDER_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/vkbuilder")
function(eve_prepare_vkbuilder_headers)
if(EXISTS "${_vkbuilder_header}")
    file(READ "${_vkbuilder_header}" _vkbuilder_source)
    if(APPLE AND EVENGINE_THIRD_PARTY_BINARY_DIR)
        set(_vkbuilder_vulkan_root "${EVENGINE_THIRD_PARTY_BINARY_DIR}/include/vulkan")
        if(NOT EXISTS "${_vkbuilder_vulkan_root}/vulkan.h" OR
           NOT EXISTS "${_vkbuilder_vulkan_root}/vulkan.hpp")
            message(FATAL_ERROR "Prebuilt third-party install is missing pinned Vulkan headers")
        endif()
        # The release prebuilt archives compile Vulkan-Hpp without its Debug-only
        # DispatchLoaderBase fields. Keep that layout in Debug consumers too;
        # otherwise an inline dispatcher call can read a function pointer 16
        # bytes before the member that Graphics initialized.
        string(CONCAT _vkbuilder_pinned_vulkan
            "#include \"${_vkbuilder_vulkan_root}/vulkan.h\"\n"
            "#if !defined(NDEBUG)\n"
            "#include <cassert>\n"
            "#pragma push_macro(\"assert\")\n"
            "#undef assert\n#define assert(...) ((void)0)\n"
            "#define NDEBUG 1\n#define EVENGINE_VKBUILDER_RESTORE_DEBUG_ASSERT 1\n"
            "#endif\n"
            "#include \"${_vkbuilder_vulkan_root}/vulkan.hpp\"\n"
            "#if defined(EVENGINE_VKBUILDER_RESTORE_DEBUG_ASSERT)\n"
            "#undef NDEBUG\n#pragma pop_macro(\"assert\")\n"
            "#undef EVENGINE_VKBUILDER_RESTORE_DEBUG_ASSERT\n"
            "#endif")
        string(REPLACE "#include <vulkan/vulkan.hpp>"
            "${_vkbuilder_pinned_vulkan}" _vkbuilder_source "${_vkbuilder_source}")
    endif()
    file(MAKE_DIRECTORY "${EVENGINE_VKBUILDER_INCLUDE_DIR}")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/third-party/VKBuilder/include/"
        DESTINATION "${EVENGINE_VKBUILDER_INCLUDE_DIR}")
    file(WRITE "${EVENGINE_VKBUILDER_INCLUDE_DIR}/vkbuilder.hpp" "${_vkbuilder_source}")
endif()
endfunction()


