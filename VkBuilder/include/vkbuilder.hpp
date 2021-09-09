#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include <dbg.h>

namespace vkb {

// Agent template for adding a default object for vkb::Instance/Device...
template<class T>
struct Agent {
  T instance;

  T* operator->() { return &instance; }
  const T* operator->() const { return &instance; }
  
  operator T() { return instance; }
  operator const T() const { return instance; }
};


#pragma region Instance
struct Instance : Agent<vk::Instance> {
  vk::DebugUtilsMessengerEXT debug_messenger;
  vk::Optional<const vk::AllocationCallbacks> allocation_callbacks = nullptr;

  bool headless = false;
  uint32_t instance_version = VK_MAKE_VERSION(1, 2, 0);

  void destroy() { instance.destroy(debug_messenger, allocation_callbacks); }
};

// Gathers useful information about the available vulkan capabilities, like
// layers and instance extensions. Use this for enabling features conditionally,
// ie if you would like an extension but can use a fallback if it isn't
// supported but need to know if support is available first.
struct SystemInfo {
private:
  SystemInfo() {}

public:
  static SystemInfo query() {
    SystemInfo info;
    info.available_layers = vk::enumerateInstanceLayerProperties();
    if (info.is_layer_available("VK_LAYER_KHRONOS_validation"))
      info.validation_layers_available = true;

    info.available_extensions = vk::enumerateInstanceExtensionProperties();
    if (info.is_extension_available("VK_EXT_debug_utils"))
      info.debug_utils_available = true;

    if (info.debug_utils_available == false)
      for (auto &layer : info.available_layers) {
        std::vector<vk::ExtensionProperties> layer_extensions =
            vk::enumerateInstanceExtensionProperties(
                std::string(layer.layerName.data()));
        for (auto &ext : layer_extensions)
          if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            info.debug_utils_available = true;
      }
    return info;
  }

  // Returns true if a layer is available
  bool is_layer_available(std::string layer_name) const {
    for (auto &layer : available_layers)
      if (layer_name == layer.layerName)
        return true;
    return false;
  }

  // Returns true if an extension is available
  bool is_extension_available(std::string extension_name) {
    for (auto &ext : available_extensions)
      if (extension_name == ext.extensionName)
        return true;
    return false;
  }

  std::vector<vk::LayerProperties> available_layers;
  std::vector<vk::ExtensionProperties> available_extensions;
  bool validation_layers_available = false;
  bool debug_utils_available = false;
};

namespace helper {

static inline const char *
to_string_message_severity(VkDebugUtilsMessageSeverityFlagBitsEXT s) {
  switch (s) {
  case VkDebugUtilsMessageSeverityFlagBitsEXT::
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    return "VERBOSE";
  case VkDebugUtilsMessageSeverityFlagBitsEXT::
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    return "ERROR";
  case VkDebugUtilsMessageSeverityFlagBitsEXT::
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    return "WARNING";
  case VkDebugUtilsMessageSeverityFlagBitsEXT::
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    return "INFO";
  default:
    return "UNKNOWN";
  }
}
static inline const char *
to_string_message_type(VkDebugUtilsMessageTypeFlagsEXT s) {
  if (s == 7)
    return "General | Validation | Performance";
  if (s == 6)
    return "Validation | Performance";
  if (s == 5)
    return "General | Performance";
  if (s == 4 /*VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/)
    return "Performance";
  if (s == 3)
    return "General | Validation";
  if (s == 2 /*VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT*/)
    return "Validation";
  if (s == 1 /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT*/)
    return "General";
  return "Unknown";
}

static inline bool
check_layer_supported(std::vector<vk::LayerProperties> const &available_layers,
                      const char *layer_name) {
  if (!layer_name)
    return false;
  for (const auto &layer_properties : available_layers) {
    if (strcmp(layer_name, layer_properties.layerName) == 0) {
      return true;
    }
  }
  return false;
}

static inline bool
check_layers_supported(std::vector<vk::LayerProperties> const &available_layers,
                       std::vector<const char *> const &layer_names) {
  bool all_found = true;
  for (const auto &layer_name : layer_names) {
    bool found = check_layer_supported(available_layers, layer_name);
    if (!found)
      all_found = false;
  }
  return all_found;
}

static inline bool check_extension_supported(
    std::vector<vk::ExtensionProperties> const &available_extensions,
    const char *extension_name) {
  if (!extension_name)
    return false;
  for (const auto &extension_properties : available_extensions) {
    if (strcmp(extension_name, extension_properties.extensionName) == 0) {
      return true;
    }
  }
  return false;
}

static inline bool check_extensions_supported(
    std::vector<vk::ExtensionProperties> const &available_extensions,
    std::vector<const char *> const &extension_names) {
  bool all_found = true;
  for (const auto extension_name : extension_names) {
    bool found =
        check_extension_supported(available_extensions, extension_name);
    if (!found)
      all_found = false;
  }
  return all_found;
}

static inline bool check_add_window_ext(const char *name,
                                        const SystemInfo &system,
                                        std::vector<const char *>& extensions) {
  if (!check_extension_supported(system.available_extensions, name))
    return false;
  extensions.push_back(name);
  return true;
};

static inline std::vector<const char *>
strvec2c_str(std::vector<std::string> strvec) {
  std::vector<const char *> result;
  for (auto s : strvec) {
    result.push_back(s.data());
  }
  return result;
}

template <typename T>
void setup_pNext_chain(T &structure,
                       const std::vector<vk::BaseOutStructure *> &structs) {
  structure.pNext = nullptr;
  if (structs.size() <= 0)
    return;
  for (size_t i = 0; i < structs.size() - 1; i++) {
    structs[i]->pNext = structs[i + 1];
  }
  structs[structs.size()-1]->pNext = nullptr;
  structure.pNext = structs.at(0);
}



extern VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

static inline vk::DebugUtilsMessengerEXT create_debug_utils_messenger(
    vk::Instance instance, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
    vk::DebugUtilsMessageSeverityFlagsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    vk::Optional<const vk::AllocationCallbacks> allocation_callbacks) {

  vk::DispatchLoaderDynamic dldi( instance, vkGetInstanceProcAddr );
  if (debug_callback == nullptr)
    debug_callback = default_debug_callback;
  vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
  messengerCreateInfo.pNext = nullptr;
  messengerCreateInfo.messageSeverity = severity;
  messengerCreateInfo.messageType = type;
  messengerCreateInfo.pfnUserCallback = debug_callback;

  return instance.createDebugUtilsMessengerEXT(messengerCreateInfo,
                                               allocation_callbacks, 
                                               dldi);
}

} // namespace helper

class InstanceBuilder {
public:
  static bool loaded;
  static void loadDefault() {
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
// vk::Instance instance = vk::createInstance({}, nullptr);
//     VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
  }

  InstanceBuilder() {
    if (!loaded) loadDefault();
  }

  // Create a VkInstance. Throw an error if it failed.
  Instance build() const {
    SystemInfo system = SystemInfo::query();

    uint32_t api_version = VK_MAKE_VERSION(1, 0, 0);

    if (required_api_version > VK_MAKE_VERSION(1, 0, 0) ||
        desired_api_version > VK_MAKE_VERSION(1, 0, 0)) {

      uint32_t queried_api_version = VK_MAKE_VERSION(1, 0, 0);
      queried_api_version = vk::enumerateInstanceVersion();

      if (queried_api_version < required_api_version) {
        throw std::runtime_error("required_api_version not satisfied");
      }
      if (required_api_version > VK_MAKE_VERSION(1, 0, 0)) {
        api_version = required_api_version;
      } else if (desired_api_version > VK_MAKE_VERSION(1, 0, 0)) {
        if (queried_api_version >= desired_api_version)
          api_version = desired_api_version;
        else
          api_version = queried_api_version;
      }
    }

    vk::ApplicationInfo app_info;
    app_info.pApplicationName = app_name.c_str();
    app_info.applicationVersion = application_version;
    app_info.pEngineName = engine_name.c_str();
    app_info.engineVersion = engine_version;
    app_info.apiVersion = api_version;

    std::vector<const char *> extensions;
    for (auto &ext : this->extensions)
      extensions.push_back(ext.c_str());
    if (debug_callback != nullptr && system.debug_utils_available) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!headless_context) {

      bool khr_surface_added =
          helper::check_add_window_ext("VK_KHR_surface", system, extensions);
#if defined(_WIN32)
      bool added_window_exts = helper::check_add_window_ext(
          "VK_KHR_win32_surface", system, extensions);
#elif defined(__ANDROID__)
      bool added_window_exts = helper::check_add_window_ext(
          "VK_KHR_android_surface", system, extensions);
#elif defined(_DIRECT2DISPLAY)
      bool added_window_exts =
          helper::check_add_window_ext("VK_KHR_display", system, extensions);
#elif defined(__linux__)
      // make sure all three calls to check_add_window_ext, don't allow short
      // circuiting
      bool added_window_exts = helper::check_add_window_ext(
          "VK_KHR_xcb_surface", system, extensions);
      added_window_exts = helper::check_add_window_ext("VK_KHR_xlib_surface",
                                                       system, extensions) ||
                          added_window_exts;
      added_window_exts = helper::check_add_window_ext("VK_KHR_wayland_surface",
                                                       system, extensions) ||
                          added_window_exts;
#elif defined(__APPLE__)
      bool added_window_exts = helper::check_add_window_ext(
          "VK_EXT_metal_surface", system, extensions);
#endif
      if (!khr_surface_added || !added_window_exts)
        throw std::runtime_error("window exts not found");
    }
    
    bool all_extensions_supported = helper::check_extensions_supported(
        system.available_extensions, extensions);
    if (!all_extensions_supported) {
      throw std::runtime_error("required extensions are not supported");
    }

    std::vector<const char *> layers;
    for (auto &layer : this->layers)
      layers.push_back(layer.c_str());

    if (m_enable_validation_layers ||
        (m_request_validation_layers && system.validation_layers_available)) {
      layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    bool all_layers_supported =
        helper::check_layers_supported(system.available_layers, layers);
    if (!all_layers_supported)
      throw std::runtime_error("not all required layer supported");

    std::vector<vk::BaseOutStructure *> pNext_chain;
    vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
    if (use_debug_messenger) {
      messengerCreateInfo.setMessageSeverity(debug_message_severity);
      messengerCreateInfo.setMessageType(debug_message_type);
      messengerCreateInfo.setPfnUserCallback(debug_callback);
      pNext_chain.push_back(
          reinterpret_cast<vk::BaseOutStructure *>(&messengerCreateInfo));
    }
    vk::ValidationFeaturesEXT features;
    if (enabled_validation_features.size() != 0 ||
        disabled_validation_features.size()) {
      features.enabledValidationFeatureCount =
          static_cast<uint32_t>(enabled_validation_features.size());
      features.pEnabledValidationFeatures = enabled_validation_features.data();
      features.disabledValidationFeatureCount =
          static_cast<uint32_t>(disabled_validation_features.size());
      features.pDisabledValidationFeatures =
          disabled_validation_features.data();
      pNext_chain.push_back(
          reinterpret_cast<vk::BaseOutStructure *>(&features));
    }

    vk::ValidationFlagsEXT checks;
    if (disabled_validation_checks.size() != 0) {
      checks.disabledValidationCheckCount =
          static_cast<uint32_t>(disabled_validation_checks.size());
      checks.pDisabledValidationChecks = disabled_validation_checks.data();
      pNext_chain.push_back(reinterpret_cast<vk::BaseOutStructure *>(&checks));
    }

    vk::InstanceCreateInfo instance_create_info;
    helper::setup_pNext_chain<vk::InstanceCreateInfo>(instance_create_info,
                                                      pNext_chain);
    instance_create_info.flags = flags;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    instance_create_info.ppEnabledExtensionNames = extensions.data();
    instance_create_info.enabledLayerCount =
        static_cast<uint32_t>(layers.size());
    instance_create_info.ppEnabledLayerNames = layers.data();

    Instance inst;
    inst.instance = vk::createInstance(instance_create_info, allocation_callbacks);
      
    VULKAN_HPP_DEFAULT_DISPATCHER.init(inst.instance);

    if (use_debug_messenger) {
      inst.debug_messenger = helper::create_debug_utils_messenger(
          inst.instance, debug_callback, debug_message_severity,
          debug_message_type, allocation_callbacks);
    }

    return inst;
  }

  // Sets the name of the application. Defaults to "" if none is provided.
  InstanceBuilder &set_app_name(std::string app_name) {
    this->app_name = app_name;
    return *this;
  }
  // Sets the name of the engine. Defaults to "" if none is provided.
  InstanceBuilder &set_engine_name(std::string engine_name) {
    this->engine_name = engine_name;
    return *this;
  }

  // Sets the (major, minor, patch) version of the application.
  InstanceBuilder &set_app_version(uint32_t major, uint32_t minor,
                                   uint32_t patch = 0) {
    this->application_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }
  // Sets the (major, minor, patch) version of the engine.
  InstanceBuilder &set_engine_version(uint32_t major, uint32_t minor,
                                      uint32_t patch = 0) {
    this->engine_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }

  // Require a vulkan instance API version. Will fail to create if this version
  // isn't available.
  InstanceBuilder &require_api_version(uint32_t major, uint32_t minor,
                                       uint32_t patch = 0) {
    this->required_api_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }

  // Prefer a vulkan instance API version. If the desired version isn't
  // available, it will use the highest version available.
  InstanceBuilder &desire_api_version(uint32_t major, uint32_t minor,
                                      uint32_t patch = 0) {
    this->desired_api_version = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }

  // Adds a layer to be enabled. Will fail to create an instance if the layer
  // isn't available.
  InstanceBuilder &enable_layer(std::string layer_name) {
    layers.push_back(layer_name);
    return *this;
  }

  // Adds an extension to be enabled. Will fail to create an instance if the
  // extension isn't available.
  InstanceBuilder &enable_extension(std::string extension_name) {
    extensions.push_back(extension_name);
    return *this;
  }

  // Headless Mode does not load the required extensions for presentation.
  // Defaults to true.
  InstanceBuilder &set_headless(bool headless = true) {
    headless_context = headless;
    return *this;
  }

  // Enables the validation layers. Will fail to create an instance if the
  // validation layers aren't available.
  InstanceBuilder &enable_validation_layers(bool require_validation = true) {
    m_enable_validation_layers = require_validation;
    return *this;
  }
  // Checks if the validation layers are available and loads them if they are.
  InstanceBuilder &request_validation_layers(bool enable_validation = true) {
    m_request_validation_layers = enable_validation;
    return *this;
  }

  // Use a default debug callback that prints to standard out.
  InstanceBuilder &use_default_debug_messenger() {
    use_debug_messenger = true;
    debug_callback = helper::default_debug_callback;
    return *this;
  }
  // Provide a user defined debug callback.
  InstanceBuilder &
  set_debug_callback(PFN_vkDebugUtilsMessengerCallbackEXT callback) {
    use_debug_messenger = true;
    debug_callback = callback;
    return *this;
  }

  // Set what message severity is needed to trigger the callback.
  InstanceBuilder &
  set_debug_messenger_severity(vk::DebugUtilsMessageSeverityFlagsEXT severity) {
    debug_message_severity = severity;
    return *this;
  }
  // Add a message severity to the list that triggers the callback.
  InstanceBuilder &
  add_debug_messenger_severity(vk::DebugUtilsMessageSeverityFlagsEXT severity) {
    debug_message_severity = debug_message_severity | severity;
    return *this;
  }
  // Set what message type triggers the callback.
  InstanceBuilder &
  set_debug_messenger_type(vk::DebugUtilsMessageTypeFlagsEXT type) {
    debug_message_type = type;
    return *this;
  }
  // Add a message type to the list of that triggers the callback.
  InstanceBuilder &
  add_debug_messenger_type(vk::DebugUtilsMessageTypeFlagsEXT type) {
    debug_message_type = debug_message_type | type;
    return *this;
  }

  // Disable some validation checks.
  // Checks: All, and Shaders
  InstanceBuilder &add_validation_disable(vk::ValidationCheckEXT check) {
    disabled_validation_checks.push_back(check);
    return *this;
  }

  // Enables optional parts of the validation layers.
  // Parts: best practices, gpu assisted, and gpu assisted reserve binding slot.
  InstanceBuilder &
  add_validation_feature_enable(vk::ValidationFeatureEnableEXT enable) {
    enabled_validation_features.push_back(enable);
    return *this;
  }

  // Disables sections of the validation layers.
  // Options: All, shaders, thread safety, api parameters, object lifetimes,
  // core checks, and unique handles.
  InstanceBuilder &
  add_validation_feature_disable(vk::ValidationFeatureDisableEXT disable) {
    disabled_validation_features.push_back(disable);
    return *this;
  }

  // Provide custom allocation callbacks.
  InstanceBuilder &set_allocation_callbacks(vk::Optional<const vk::AllocationCallbacks> callbacks) {
    allocation_callbacks = callbacks;
    return *this;
  }

private:
  // VkApplicationInfo
  std::string app_name;
  std::string engine_name;
  uint32_t application_version = 0;
  uint32_t engine_version = 0;
  uint32_t required_api_version = VK_MAKE_VERSION(1, 0, 0);
  uint32_t desired_api_version = VK_MAKE_VERSION(1, 2, 0);

  // VkInstanceCreateInfo
  std::vector<std::string> layers;
  std::vector<std::string> extensions;
  vk::InstanceCreateFlags flags;
  std::vector<VkBaseOutStructure *> pNext_elements;

  bool m_enable_validation_layers = false;
  bool m_request_validation_layers = false;
  bool use_debug_messenger = false;
  bool headless_context = false;

  // debug callback
  PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = nullptr;
  vk::DebugUtilsMessageSeverityFlagsEXT debug_message_severity = 
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  vk::DebugUtilsMessageTypeFlagsEXT debug_message_type = 
		    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;


  // validation features
  std::vector<vk::ValidationCheckEXT> disabled_validation_checks;
  std::vector<vk::ValidationFeatureEnableEXT> enabled_validation_features;
  std::vector<vk::ValidationFeatureDisableEXT> disabled_validation_features;

  // Custom allocator
  vk::Optional<const vk::AllocationCallbacks> allocation_callbacks = nullptr;
};

#pragma endregion

#pragma region PhysicalDevice
// ---- Physical Device ---- //
class PhysicalDeviceSelector;
class DeviceBuilder;

constexpr uint32_t QUEUE_INDEX_MAX_VALUE = 65536;

struct QueueFamilies {
  QueueFamilies(const std::vector<vk::QueueFamilyProperties> &families)
      : families(families) {}
  QueueFamilies() {}
  virtual ~QueueFamilies() {}

  // TODO: Support MultiQueue Parallel Workload

  // finds the first queue which supports graphics operations. returns
  // QUEUE_INDEX_MAX_VALUE if none is found
  uint32_t get_graphics_queue_index() const {
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
      if (families[i].queueFlags & vk::QueueFlagBits::eGraphics)
        return i;
    }
    return QUEUE_INDEX_MAX_VALUE;
  }

  // finds the first queue which supports only compute (not graphics or
  // transfer). returns QUEUE_INDEX_MAX_VALUE if none is found
  uint32_t get_dedicated_compute_queue_index() const {
    for (uint32_t i = 0; i < families.size(); i++) {
      if ((families[i].queueFlags & vk::QueueFlagBits::eCompute) &&
          !(families[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
          !(families[i].queueFlags & vk::QueueFlagBits::eTransfer))
        return i;
    }
    return QUEUE_INDEX_MAX_VALUE;
  }

  // finds a compute queue which is separate from the graphics queue and tries
  // to find one without transfer support returns QUEUE_INDEX_MAX_VALUE if none
  // is found
  uint32_t get_separate_compute_queue_index() const {
    uint32_t compute = QUEUE_INDEX_MAX_VALUE;
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
      if ((families[i].queueFlags & vk::QueueFlagBits::eCompute) &&
          !(families[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
        if (!(families[i].queueFlags & vk::QueueFlagBits::eTransfer)) {
          return i;
        } else {
          compute = i;
        }
      }
    }
    return compute;
  }

  // finds a transfer queue which is separate from the graphics queue and tries
  // to find one without compute support returns QUEUE_INDEX_MAX_VALUE if none
  // is found
  uint32_t get_separate_transfer_queue_index() const {
    uint32_t transfer = QUEUE_INDEX_MAX_VALUE;
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
      if ((families[i].queueFlags & vk::QueueFlagBits::eTransfer) &&
          !(families[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
        if (!(families[i].queueFlags & vk::QueueFlagBits::eCompute)) {
          return i;
        } else {
          transfer = i;
        }
      }
    }
    return transfer;
  }

  // finds the first queue which supports only transfer (not graphics or
  // compute). returns QUEUE_INDEX_MAX_VALUE if none is found
  uint32_t get_dedicated_transfer_queue_index() const {
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
      if ((families[i].queueFlags & vk::QueueFlagBits::eTransfer) &&
          !(families[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
          !(families[i].queueFlags & vk::QueueFlagBits::eCompute))
        return i;
    }
    return QUEUE_INDEX_MAX_VALUE;
  }
  // finds the first queue which supports presenting. returns
  // QUEUE_INDEX_MAX_VALUE if none is found
  uint32_t get_present_queue_index(vk::PhysicalDevice const phys_device,
                                   vk::SurfaceKHR const surface) const {
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); i++) {
      VkBool32 presentSupport = false;
      if (surface) {
        vk::Result res =
            phys_device.getSurfaceSupportKHR(i, surface, &presentSupport);
        if (res != vk::Result::eSuccess)
          return QUEUE_INDEX_MAX_VALUE; // TODO: determine if this should fail
                                        // another way
      }
      if (presentSupport == VK_TRUE)
        return i;
    }
    return QUEUE_INDEX_MAX_VALUE;
  }

  std::vector<vk::QueueFamilyProperties> families;
};

struct PhysicalDevice : Agent<vk::PhysicalDevice>{
  vk::SurfaceKHR surface;

  vk::PhysicalDeviceFeatures features;
  vk::PhysicalDeviceProperties properties;
  vk::PhysicalDeviceMemoryProperties memory_properties;

  // Has a queue family that supports compute operations but not graphics nor
  // transfer.
  bool has_dedicated_compute_queue() const {
    return queue_families.get_dedicated_compute_queue_index() !=
           QUEUE_INDEX_MAX_VALUE;
  }
  // Has a queue family that supports transfer operations but not graphics nor
  // compute.
  bool has_dedicated_transfer_queue() const {
    return queue_families.get_dedicated_transfer_queue_index() !=
           QUEUE_INDEX_MAX_VALUE;
  }

  // Has a queue family that supports transfer operations but not graphics.
  bool has_separate_compute_queue() const {
    return queue_families.get_separate_compute_queue_index() !=
           QUEUE_INDEX_MAX_VALUE;
  }
  // Has a queue family that supports transfer operations but not graphics.
  bool has_separate_transfer_queue() const {
    return queue_families.get_separate_transfer_queue_index() !=
           QUEUE_INDEX_MAX_VALUE;
  }

  // Advanced: Get the VkQueueFamilyProperties of the device if special queue
  // setup is needed
  QueueFamilies get_queue_families() const { return queue_families; }

  uint32_t findMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
    vk::PhysicalDeviceMemoryProperties memProperties = instance.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && 
          (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }

    throw std::runtime_error("failed to find suitable memory type!");
    return 0;
  }

private:
  std::vector<std::string> extensions_to_enable;
  QueueFamilies queue_families;
  bool defer_surface_initialization = false;
  friend class PhysicalDeviceSelector;
  friend class DeviceBuilder;
};

enum class PreferredDeviceType {
  other = 0,
  integrated = 1,
  discrete = 2,
  virtual_gpu = 3,
  cpu = 4
};

class PhysicalDeviceSelector {
public:
  // Requires a Instance to construct, needed to pass instance creation
  // info.
  PhysicalDeviceSelector(Instance const &instance) {
    system_info.instance = instance.instance;
    system_info.headless = instance.headless;
    criteria.require_present = !instance.headless;
    criteria.required_version = instance.instance_version;
    criteria.desired_version = instance.instance_version;
  }

  PhysicalDevice select() const {
    if (!system_info.headless && !criteria.defer_surface_initialization) {
      if (!system_info.surface)
        throw std::runtime_error("no_surface_provided");
    }

    auto physical_devices = system_info.instance.enumeratePhysicalDevices();
    if (physical_devices.empty()) {
      throw std::runtime_error("failed_enumerate_physical_devices");
    }
    if (physical_devices.size() == 0) {
      throw std::runtime_error("no_physical_devices_found");
    }

    std::vector<PhysicalDeviceDesc> phys_device_descriptions;
    for (auto &phys_device : physical_devices) {
      phys_device_descriptions.push_back(populate_device_details(phys_device));
    }

    PhysicalDeviceDesc selected_device{};

    if (criteria.use_first_gpu_unconditionally) {
      selected_device = phys_device_descriptions.at(0);
    } else {
      for (const auto &device : phys_device_descriptions) {
        auto suitable = is_device_suitable(device);
        if (suitable == Suitable::yes) {
          selected_device = device;
          break;
        } else if (suitable == Suitable::partial) {
          selected_device = device;
        }
      }
    }
   
    if (!selected_device.phys_device) {
      throw std::runtime_error("no_suitable_device");
    }
 
    PhysicalDevice out_device;
    out_device.instance = selected_device.phys_device;
    out_device.surface = system_info.surface;
    out_device.features = criteria.required_features;
    out_device.properties = selected_device.device_properties;
    out_device.memory_properties = selected_device.mem_properties;
    out_device.queue_families = selected_device.queue_families;
    out_device.defer_surface_initialization =
        criteria.defer_surface_initialization;

    out_device.extensions_to_enable.insert(
        out_device.extensions_to_enable.end(),
        criteria.required_extensions.begin(),
        criteria.required_extensions.end());
    auto desired_extensions_supported =
        selected_device.check_device_extension_support(
            criteria.desired_extensions);
    out_device.extensions_to_enable.insert(
        out_device.extensions_to_enable.end(),
        desired_extensions_supported.begin(),
        desired_extensions_supported.end());
    return out_device;
  }

  // Set the surface in which the physical device should render to.
  PhysicalDeviceSelector &set_surface(vk::SurfaceKHR surface) {
    system_info.surface = surface;
    system_info.headless = false;
    return *this;
  }

  // Set the desired physical device type to select. Defaults to
  // PreferredDeviceType::discrete.
  PhysicalDeviceSelector &prefer_gpu_device_type(
      PreferredDeviceType type = PreferredDeviceType::discrete) {
    criteria.preferred_type = type;
    return *this;
  }

  // Allow selection of a gpu device type that isn't the preferred physical
  // device type. Defaults to true.
  PhysicalDeviceSelector &
  allow_any_gpu_device_type(bool allow_any_type = true) {
    criteria.allow_any_type = allow_any_type;
    return *this;
  }

  // Require that a physical device supports presentation. Defaults to true.
  PhysicalDeviceSelector &require_present(bool require = true) {
    criteria.require_present = require;
    return *this;
  }

  // Require a queue family that supports compute operations but not graphics
  // nor transfer.
  PhysicalDeviceSelector &require_dedicated_compute_queue() {
    criteria.require_dedicated_compute_queue = true;
    return *this;
  }

  // Require a queue family that supports transfer operations but not graphics
  // nor compute.
  PhysicalDeviceSelector &require_dedicated_transfer_queue() {
    criteria.require_dedicated_transfer_queue = true;
    return *this;
  }

  // Require a queue family that supports compute operations but not graphics.
  PhysicalDeviceSelector &require_separate_compute_queue() {
    criteria.require_separate_compute_queue = true;
    return *this;
  }

  // Require a queue family that supports transfer operations but not graphics.
  PhysicalDeviceSelector &require_separate_transfer_queue() {
    criteria.require_separate_transfer_queue = true;
    return *this;
  }

  // Require a memory heap from VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT with `size`
  // memory available.
  PhysicalDeviceSelector &required_device_memory_size(vk::DeviceSize size) {
    criteria.required_mem_size = size;
    return *this;
  }
  // Prefer a memory heap from VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT with `size`
  // memory available.
  PhysicalDeviceSelector &desired_device_memory_size(vk::DeviceSize size) {
    criteria.desired_mem_size = size;
    return *this;
  }

  // Require a physical device which supports a specific extension.
  PhysicalDeviceSelector &add_required_extension(std::string extension) {
    criteria.required_extensions.push_back(extension);
    return *this;
  }
  // Require a physical device which supports a set of extensions.
  PhysicalDeviceSelector &
  add_required_extensions(std::vector<std::string> extensions) {
    criteria.required_extensions.insert(criteria.required_extensions.end(),
                                        extensions.begin(), extensions.end());
    return *this;
  }

  // Prefer a physical device which supports a specific extension.
  PhysicalDeviceSelector &add_desired_extension(std::string extension) {
    criteria.desired_extensions.push_back(extension);
    return *this;
  }
  // Prefer a physical device which supports a set of extensions.
  PhysicalDeviceSelector &
  add_desired_extensions(std::vector<std::string> extensions) {
    criteria.desired_extensions.insert(criteria.desired_extensions.end(),
                                       extensions.begin(), extensions.end());
    return *this;
  }

  // Prefer a physical device that supports a (major, minor) version of vulkan.
  PhysicalDeviceSelector &set_desired_version(uint32_t major, uint32_t minor) {
    criteria.desired_version = VK_MAKE_VERSION(major, minor, 0);
    return *this;
  }
  // Require a physical device that supports a (major, minor) version of vulkan.
  PhysicalDeviceSelector &set_minimum_version(uint32_t major, uint32_t minor) {
    criteria.required_version = VK_MAKE_VERSION(major, minor, 0);
    return *this;
  }

  // Require a physical device which supports the features in
  // VkPhysicalDeviceFeatures.
  PhysicalDeviceSelector &
  set_required_features(vk::PhysicalDeviceFeatures features) {
    criteria.required_features = features;
    return *this;
  }

  // Used when surface creation happens after physical device selection.
  // Warning: This disables checking if the physical device supports a given
  // surface.
  PhysicalDeviceSelector &defer_surface_initialization() {
    criteria.defer_surface_initialization = true;
    return *this;
  }

  // Ignore all criteria and choose the first physical device that is available.
  // Only use when: The first gpu in the list may be set by global user
  // preferences and an application may wish to respect it.
  PhysicalDeviceSelector &
  select_first_device_unconditionally(bool unconditionally = true) {
    criteria.use_first_gpu_unconditionally = unconditionally;
    return *this;
  }

private:
  struct SystemInfo {
    vk::Instance instance;
    vk::SurfaceKHR surface;
    bool headless = false;
  } system_info;

  struct PhysicalDeviceDesc {
    vk::PhysicalDevice phys_device;
    QueueFamilies queue_families;

    vk::PhysicalDeviceFeatures device_features;
    vk::PhysicalDeviceProperties device_properties;
    vk::PhysicalDeviceMemoryProperties mem_properties;

    std::vector<std::string> check_device_extension_support(
        std::vector<std::string> desired_extensions) {
      auto available_extensions =
          phys_device.enumerateDeviceExtensionProperties();

      std::vector<std::string> extensions_to_enable;
      for (const auto &extension : available_extensions) {
        for (auto &req_ext : desired_extensions) {
          if (req_ext == extension.extensionName) {
            extensions_to_enable.push_back(req_ext);
            break;
          }
        }
      }
      return extensions_to_enable;
    }
  };

  PhysicalDeviceDesc
  populate_device_details(vk::PhysicalDevice phys_device) const {
    PhysicalDeviceSelector::PhysicalDeviceDesc desc{};
    desc.phys_device = phys_device;
    desc.queue_families = QueueFamilies(phys_device.getQueueFamilyProperties());

    desc.device_properties = phys_device.getProperties();
    desc.device_features = phys_device.getFeatures();
    desc.mem_properties = phys_device.getMemoryProperties();
    return desc;
  }

  struct SelectionCriteria {
    PreferredDeviceType preferred_type = PreferredDeviceType::discrete;
    bool allow_any_type = true;
    bool require_present = true;
    bool require_dedicated_transfer_queue = false;
    bool require_dedicated_compute_queue = false;
    bool require_separate_transfer_queue = false;
    bool require_separate_compute_queue = false;
    vk::DeviceSize required_mem_size = 0;
    vk::DeviceSize desired_mem_size = 0;

    std::vector<std::string> required_extensions;
    std::vector<std::string> desired_extensions;

    uint32_t required_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t desired_version = VK_MAKE_VERSION(1, 0, 0);

    vk::PhysicalDeviceFeatures required_features;

    bool defer_surface_initialization = false;
    bool use_first_gpu_unconditionally = false;
  } criteria;

  enum class Suitable { yes, partial, no };

  Suitable is_device_suitable(PhysicalDeviceDesc pd) const {
    Suitable suitable = Suitable::yes;

    if (criteria.required_version > pd.device_properties.apiVersion)
      return Suitable::no;
    if (criteria.desired_version > pd.device_properties.apiVersion)
      suitable = Suitable::partial;

    bool dedicated_compute =
        pd.queue_families.get_dedicated_compute_queue_index() !=
        QUEUE_INDEX_MAX_VALUE;
    bool dedicated_transfer =
        pd.queue_families.get_dedicated_transfer_queue_index() !=
        QUEUE_INDEX_MAX_VALUE;
    bool separate_compute =
        pd.queue_families.get_separate_compute_queue_index() !=
        QUEUE_INDEX_MAX_VALUE;
    bool separate_transfer =
        pd.queue_families.get_separate_transfer_queue_index() !=
        QUEUE_INDEX_MAX_VALUE;

    bool present_queue =
        pd.queue_families.get_present_queue_index(
            pd.phys_device, system_info.surface) != QUEUE_INDEX_MAX_VALUE;

    if (criteria.require_dedicated_compute_queue && !dedicated_compute)
      return Suitable::no;
    if (criteria.require_dedicated_transfer_queue && !dedicated_transfer)
      return Suitable::no;
    if (criteria.require_separate_compute_queue && !separate_compute)
      return Suitable::no;
    if (criteria.require_separate_transfer_queue && !separate_transfer)
      return Suitable::no;
    if (criteria.require_present && !present_queue &&
        !criteria.defer_surface_initialization)
      return Suitable::no;

    auto required_extensions_supported =
        pd.check_device_extension_support(criteria.required_extensions);
    if (required_extensions_supported.size() !=
        criteria.required_extensions.size())
      return Suitable::no;

    auto desired_extensions_supported =
        pd.check_device_extension_support(criteria.desired_extensions);
    if (desired_extensions_supported.size() !=
        criteria.desired_extensions.size())
      suitable = Suitable::partial;

    bool swapChainAdequate = false;
    if (criteria.defer_surface_initialization) {
      swapChainAdequate = true;
    } else if (!system_info.headless) {

      auto formats = pd.phys_device.getSurfaceFormatsKHR(system_info.surface);
      auto present_modes =
          pd.phys_device.getSurfacePresentModesKHR(system_info.surface);

      if (!formats.empty() && !present_modes.empty()) {
        swapChainAdequate = true;
      }
    }
    if (criteria.require_present && !swapChainAdequate)
      return Suitable::no;

    if (pd.device_properties.deviceType !=
        static_cast<vk::PhysicalDeviceType>(criteria.preferred_type)) {
      if (criteria.allow_any_type)
        suitable = Suitable::partial;
      else
        return Suitable::no;
    }

    bool required_features_supported =
        supports_features(pd.device_features, criteria.required_features);
    if (!required_features_supported)
      return Suitable::no;

    bool has_required_memory = false;
    bool has_preferred_memory = false;
    for (uint32_t i = 0; i < pd.mem_properties.memoryHeapCount; i++) {
      if (pd.mem_properties.memoryHeaps[i].flags &
          vk::MemoryHeapFlagBits::eDeviceLocal) {
        if (pd.mem_properties.memoryHeaps[i].size >
            criteria.required_mem_size) {
          has_required_memory = true;
        }
        if (pd.mem_properties.memoryHeaps[i].size > criteria.desired_mem_size) {
          has_preferred_memory = true;
        }
      }
    }
    if (!has_required_memory)
      return Suitable::no;
    if (!has_preferred_memory)
      suitable = Suitable::partial;

    return suitable;
  }

  static bool supports_features(vk::PhysicalDeviceFeatures supported,
                                vk::PhysicalDeviceFeatures requested) {
    // clang-format off
        if (requested.robustBufferAccess && !supported.robustBufferAccess) return false;
        if (requested.fullDrawIndexUint32 && !supported.fullDrawIndexUint32) return false;
        if (requested.imageCubeArray && !supported.imageCubeArray) return false;
        if (requested.independentBlend && !supported.independentBlend) return false;
        if (requested.geometryShader && !supported.geometryShader) return false;
        if (requested.tessellationShader && !supported.tessellationShader) return false;
        if (requested.sampleRateShading && !supported.sampleRateShading) return false;
        if (requested.dualSrcBlend && !supported.dualSrcBlend) return false;
        if (requested.logicOp && !supported.logicOp) return false;
        if (requested.multiDrawIndirect && !supported.multiDrawIndirect) return false;
        if (requested.drawIndirectFirstInstance && !supported.drawIndirectFirstInstance) return false;
        if (requested.depthClamp && !supported.depthClamp) return false;
        if (requested.depthBiasClamp && !supported.depthBiasClamp) return false;
        if (requested.fillModeNonSolid && !supported.fillModeNonSolid) return false;
        if (requested.depthBounds && !supported.depthBounds) return false;
        if (requested.wideLines && !supported.wideLines) return false;
        if (requested.largePoints && !supported.largePoints) return false;
        if (requested.alphaToOne && !supported.alphaToOne) return false;
        if (requested.multiViewport && !supported.multiViewport) return false;
        if (requested.samplerAnisotropy && !supported.samplerAnisotropy) return false;
        if (requested.textureCompressionETC2 && !supported.textureCompressionETC2) return false;
        if (requested.textureCompressionASTC_LDR && !supported.textureCompressionASTC_LDR) return false;
        if (requested.textureCompressionBC && !supported.textureCompressionBC) return false;
        if (requested.occlusionQueryPrecise && !supported.occlusionQueryPrecise) return false;
        if (requested.pipelineStatisticsQuery && !supported.pipelineStatisticsQuery) return false;
        if (requested.vertexPipelineStoresAndAtomics && !supported.vertexPipelineStoresAndAtomics) return false;
        if (requested.fragmentStoresAndAtomics && !supported.fragmentStoresAndAtomics) return false;
        if (requested.shaderTessellationAndGeometryPointSize && !supported.shaderTessellationAndGeometryPointSize) return false;
        if (requested.shaderImageGatherExtended && !supported.shaderImageGatherExtended) return false;
        if (requested.shaderStorageImageExtendedFormats && !supported.shaderStorageImageExtendedFormats) return false;
        if (requested.shaderStorageImageMultisample && !supported.shaderStorageImageMultisample) return false;
        if (requested.shaderStorageImageReadWithoutFormat && !supported.shaderStorageImageReadWithoutFormat) return false;
        if (requested.shaderStorageImageWriteWithoutFormat && !supported.shaderStorageImageWriteWithoutFormat) return false;
        if (requested.shaderUniformBufferArrayDynamicIndexing && !supported.shaderUniformBufferArrayDynamicIndexing) return false;
        if (requested.shaderSampledImageArrayDynamicIndexing && !supported.shaderSampledImageArrayDynamicIndexing) return false;
        if (requested.shaderStorageBufferArrayDynamicIndexing && !supported.shaderStorageBufferArrayDynamicIndexing) return false;
        if (requested.shaderStorageImageArrayDynamicIndexing && !supported.shaderStorageImageArrayDynamicIndexing) return false;
        if (requested.shaderClipDistance && !supported.shaderClipDistance) return false;
        if (requested.shaderCullDistance && !supported.shaderCullDistance) return false;
        if (requested.shaderFloat64 && !supported.shaderFloat64) return false;
        if (requested.shaderInt64 && !supported.shaderInt64) return false;
        if (requested.shaderInt16 && !supported.shaderInt16) return false;
        if (requested.shaderResourceResidency && !supported.shaderResourceResidency) return false;
        if (requested.shaderResourceMinLod && !supported.shaderResourceMinLod) return false;
        if (requested.sparseBinding && !supported.sparseBinding) return false;
        if (requested.sparseResidencyBuffer && !supported.sparseResidencyBuffer) return false;
        if (requested.sparseResidencyImage2D && !supported.sparseResidencyImage2D) return false;
        if (requested.sparseResidencyImage3D && !supported.sparseResidencyImage3D) return false;
        if (requested.sparseResidency2Samples && !supported.sparseResidency2Samples) return false;
        if (requested.sparseResidency4Samples && !supported.sparseResidency4Samples) return false;
        if (requested.sparseResidency8Samples && !supported.sparseResidency8Samples) return false;
        if (requested.sparseResidency16Samples && !supported.sparseResidency16Samples) return false;
        if (requested.sparseResidencyAliased && !supported.sparseResidencyAliased) return false;
        if (requested.variableMultisampleRate && !supported.variableMultisampleRate) return false;
        if (requested.inheritedQueries && !supported.inheritedQueries) return false;
    // clang-format on
    return true;
  }
};

#pragma endregion

#pragma region Device

enum class QueueType { present, graphics, compute, transfer };

struct Device : Agent<vk::Device> {
  PhysicalDevice physical_device;
  vk::SurfaceKHR surface;
  QueueFamilies queue_families;
  vk::Optional<const vk::AllocationCallbacks> allocation_callbacks = nullptr;

  uint32_t get_queue_index(QueueType type) const {
    uint32_t index = QUEUE_INDEX_MAX_VALUE;
    switch (type) {
    case QueueType::present:
      index = queue_families.get_present_queue_index(
          physical_device, surface);
      break;
    case QueueType::graphics:
      index = queue_families.get_graphics_queue_index();
      break;
    case QueueType::compute:
      index = queue_families.get_separate_compute_queue_index();
      break;
    case QueueType::transfer:
      index = queue_families.get_separate_transfer_queue_index();
      break;
    default:
      break;
    }
    return index;
  }
  // Only a compute or transfer queue type is valid. All other queue types do
  // not support a 'dedicated' queue index
  uint32_t get_dedicated_queue_index(QueueType type) const {
    uint32_t index = QUEUE_INDEX_MAX_VALUE;
    switch (type) {
    case QueueType::compute:
      index = queue_families.get_dedicated_compute_queue_index();
      break;
    case QueueType::transfer:
      index = queue_families.get_dedicated_transfer_queue_index();
      break;
    default:
      break;
    }
    return index;
  }
  vk::Queue getQueue(uint32_t family, uint32_t queueIndex = 0) const {
    return instance.getQueue(family, queueIndex);
  }

  // This function will return a empty Queue if the type is invalid.
  vk::Queue getQueue(QueueType type, uint32_t queueIndex = 0) const {
    uint32_t family_index = get_queue_index(type);
    if (family_index == QUEUE_INDEX_MAX_VALUE)
      return vk::Queue();
    return getQueue(family_index, queueIndex);
  }
  // Only a compute or transfer queue type is valid. All other queue types do
  // not support a 'dedicated' queue
  vk::Queue get_dedicated_queue(QueueType type, uint32_t queueIndex = 0) const {
    uint32_t family_index = get_dedicated_queue_index(type);
    if (family_index == QUEUE_INDEX_MAX_VALUE)
      return vk::Queue();
    return getQueue(family_index, queueIndex);
  }

  vk::CommandPool createCommandPool(QueueType qt = QueueType::graphics) {
    vk::CommandPoolCreateInfo pool_info = {};
    pool_info.queueFamilyIndex = get_queue_index(qt);
    pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    return instance.createCommandPool(pool_info, allocation_callbacks);
  }

  std::vector<vk::CommandBuffer> createCommandBuffers(vk::CommandPool pool, uint32_t count = 1) {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool                 = pool;
    allocInfo.level                       = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount          = count;

    return instance.allocateCommandBuffers(allocInfo);
  }

  vk::Semaphore createSemaphore() {
    vk::SemaphoreCreateInfo semaphore_info = {};
    return instance.createSemaphore(semaphore_info);
  }

  std::vector<vk::Semaphore> createSemaphores(uint32_t count) {
    std::vector<vk::Semaphore> semaphores(count);
    for (uint32_t i = 0; i < count; ++i) {
      semaphores[i] = createSemaphore();
    }
    return semaphores;
  }

  vk::Fence createFence() {
    vk::FenceCreateInfo fence_info = {};
    fence_info.flags = vk::FenceCreateFlagBits::eSignaled;
    return instance.createFence(fence_info);
  }

  std::vector<vk::Fence> createFences(uint32_t count) {
    std::vector<vk::Fence> fences(count);
    for (uint32_t i = 0; i < count; ++i) {
      fences[i] = createFence();
    }
    return fences;
  }
  
  void destroy() { instance.destroy(allocation_callbacks); }
};

// For advanced device queue setup
struct CustomQueueDescription {
  explicit CustomQueueDescription(uint32_t index, uint32_t count,
                                  std::vector<float> priorities)
      : index(index), count(count), priorities(priorities) {
    assert(count == priorities.size());
  }
  uint32_t index = 0;
  uint32_t count = 0;
  std::vector<float> priorities;
};

class DeviceBuilder {
public:
  // Any features and extensions that are requested/required in
  // PhysicalDeviceSelector are automatically enabled.
  DeviceBuilder(PhysicalDevice phys_device) {
    info.physical_device = phys_device;
    info.surface = phys_device.surface;
    info.queue_families = phys_device.queue_families;
    info.features = phys_device.features;
    info.extensions_to_enable = phys_device.extensions_to_enable;
    info.defer_surface_initialization =
        phys_device.defer_surface_initialization;
  }

  // For Advanced Users: specify the exact list of VkDeviceQueueCreateInfo's
  // needed for the application. If a custom queue setup is provided, getting
  // the queues and queue indexes is up to the application.
  DeviceBuilder &
  custom_queue_setup(std::vector<CustomQueueDescription> queue_descriptions) {
    info.queue_descriptions = queue_descriptions;
    return *this;
  }

  // Add a structure to the pNext chain of VkDeviceCreateInfo.
  // The structure must be valid when DeviceBuilder::build() is called.
  template <typename T> DeviceBuilder &add_pNext(T *structure) {
    info.pNext_chain.push_back(
        reinterpret_cast<vk::BaseOutStructure *>(structure));
    return *this;
  }

  // Provide custom allocation callbacks.
  DeviceBuilder &set_allocation_callbacks(vk::Optional<const vk::AllocationCallbacks> callbacks) {
    info.allocation_callbacks = callbacks;
    return *this;
  }

  Device build() const {

    std::vector<CustomQueueDescription> queue_descriptions;
    queue_descriptions.insert(queue_descriptions.end(),
                              info.queue_descriptions.begin(),
                              info.queue_descriptions.end());

    if (queue_descriptions.size() == 0) {
      for (uint32_t i = 0; i < info.queue_families.families.size(); i++) {
        queue_descriptions.push_back(
            CustomQueueDescription{i, 1, std::vector<float>{1.0f}});
      }
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for (auto &desc : queue_descriptions) {
      vk::DeviceQueueCreateInfo queue_create_info = {};
      queue_create_info.queueFamilyIndex = desc.index;
      queue_create_info.queueCount = desc.count;
      queue_create_info.pQueuePriorities = desc.priorities.data();
      queueCreateInfos.push_back(queue_create_info);
    }

    std::vector<const char *> extensions =
        helper::strvec2c_str(info.extensions_to_enable);
    if (info.surface || info.defer_surface_initialization)
      extensions.push_back({VK_KHR_SWAPCHAIN_EXTENSION_NAME});

    // VUID-VkDeviceCreateInfo-pNext-00373 - don't add pEnabledFeatures if the
    // phys_dev_features_2 is present
    bool has_phys_dev_features_2 = false;
    for (auto &pNext_struct : info.pNext_chain) {
      if (pNext_struct->sType == vk::StructureType::ePhysicalDeviceFeatures2) {
        has_phys_dev_features_2 = true;
      }
    }

    vk::DeviceCreateInfo device_create_info = {};
    helper::setup_pNext_chain<vk::DeviceCreateInfo>(device_create_info,
                                                    info.pNext_chain);
    device_create_info.flags = info.flags;
    device_create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    device_create_info.pQueueCreateInfos = queueCreateInfos.data();
    device_create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    device_create_info.ppEnabledExtensionNames = extensions.data();
    if (!has_phys_dev_features_2) {
      device_create_info.pEnabledFeatures = &info.features;
    }
    Device device;
    auto vkdev = info.physical_device->createDevice(
        device_create_info, info.allocation_callbacks);
    if (!vkdev) {
      throw std::runtime_error("Device create failed");
    }
    device.instance = vkdev;
    device.physical_device = info.physical_device;
    device.surface = info.surface;
    device.queue_families = info.queue_families;
    device.allocation_callbacks = info.allocation_callbacks;
    return device;
  }

private:
  struct DeviceInfo {
    vk::DeviceCreateFlags flags;
    std::vector<vk::BaseOutStructure *> pNext_chain;
    PhysicalDevice physical_device;
    vk::SurfaceKHR surface;
    bool defer_surface_initialization = false;
    QueueFamilies queue_families;
    vk::PhysicalDeviceFeatures features;
    std::vector<std::string> extensions_to_enable;
    std::vector<CustomQueueDescription> queue_descriptions;
    vk::Optional<const vk::AllocationCallbacks> allocation_callbacks = nullptr;
  } info;
};

#pragma endregion

#pragma region Swapchain

struct Swapchain : Agent<vk::SwapchainKHR> {
  vk::Device device;
  uint32_t image_count = 0;
  vk::Format image_format;
  vk::Extent2D extent = {0, 0};
  vk::Optional<const vk::AllocationCallbacks> allocation_callbacks = nullptr;

  std::vector<vk::Image> swapchain_images;
  std::vector<vk::ImageView> swapchain_imageviews;
  size_t current_frame = 0;

  vk::Image getCurrentImage() {
    return swapchain_images[current_frame];
  }
  vk::ImageView getCurrentImageView() {
    return swapchain_imageviews[current_frame];
  }

  // Returns a vector of VkImage handles to the swapchain.
  std::vector<vk::Image>& get_images() {
    if (swapchain_images.empty()) {
      swapchain_images = device.getSwapchainImagesKHR(instance);
    }
    return swapchain_images;
  }

  // Returns a vector of VkImageView's to the VkImage's of the swapchain.
  // VkImageViews must be destroyed.
  std::vector<vk::ImageView>& get_image_views() {
    if (!swapchain_imageviews.empty()) return swapchain_imageviews;
    
    auto& images = get_images();
    if (images.empty())
      throw std::runtime_error("cannot get swapchain images");

    swapchain_imageviews.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
      vk::ImageViewCreateInfo createInfo;
      createInfo.image = images[i];
      createInfo.viewType = vk::ImageViewType::e2D;
      createInfo.format = image_format;
      createInfo.components.r = vk::ComponentSwizzle::eIdentity;
      createInfo.components.g = vk::ComponentSwizzle::eIdentity;
      createInfo.components.b = vk::ComponentSwizzle::eIdentity;
      createInfo.components.a = vk::ComponentSwizzle::eIdentity;
      createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;
      swapchain_imageviews[i] = device.createImageView(createInfo, allocation_callbacks);
      if (!swapchain_imageviews[i])
        throw std::runtime_error("failed_create_swapchain_image_views");
    }
    return swapchain_imageviews;
  }

  void destroy_imageviews() {
    for (auto &imageview : swapchain_imageviews) {
      device.destroyImageView(imageview, allocation_callbacks);
    }
  }

  void destroy() {
    if (device && instance) {
      if (!swapchain_imageviews.empty()) destroy_imageviews();
      device.destroySwapchainKHR(instance, allocation_callbacks);
    }
  }

  std::vector<vk::Framebuffer> createFramebuffers(vk::RenderPass render_pass) {
    auto& image_views = get_image_views();
    if (image_views.empty())
      throw std::runtime_error("cannot get swapchain image views");

    std::vector<vk::Framebuffer> swapchain_framebuffers(image_views.size());

    for (size_t i = 0; i < image_views.size(); i++) {
      vk::ImageView attachments[] = {image_views[i]};

      // TODO: Add more configuration for framebuffer
      vk::FramebufferCreateInfo framebuffer_info = {};
      framebuffer_info.renderPass              = render_pass;
      framebuffer_info.attachmentCount         = 1;
      framebuffer_info.pAttachments            = attachments;
      framebuffer_info.width                   = extent.width;
      framebuffer_info.height                  = extent.height;
      framebuffer_info.layers                  = 1;

      swapchain_framebuffers[i] = device.createFramebuffer(framebuffer_info, allocation_callbacks);
    }
    return swapchain_framebuffers;
  }
};

class SwapchainBuilder {
public:
  explicit SwapchainBuilder(Device const &device) {
    info.device = device;
    info.physical_device = device.physical_device;
    info.surface = device.surface;
    auto present = device.get_queue_index(QueueType::present);
    auto graphics = device.get_queue_index(QueueType::graphics);
    // TODO: handle error of queue's not available
    info.graphics_queue_index = present;
    info.present_queue_index = graphics;
    info.allocation_callbacks = device.allocation_callbacks;
  }
  explicit SwapchainBuilder(Device const &device,
                            vk::SurfaceKHR const surface) {
    info.device = device;
    info.physical_device = device.physical_device;
    info.surface = surface;
    Device temp_device = device;
    temp_device.surface = surface;
    auto present = temp_device.get_queue_index(QueueType::present);
    auto graphics = temp_device.get_queue_index(QueueType::graphics);
    // TODO: handle error of queue's not available
    info.graphics_queue_index = present;
    info.present_queue_index = graphics;
    info.allocation_callbacks = device.allocation_callbacks;
  }
  explicit SwapchainBuilder(
      vk::PhysicalDevice const physical_device, vk::Device const device,
      vk::SurfaceKHR const surface,
      uint32_t graphics_queue_index = QUEUE_INDEX_MAX_VALUE,
      uint32_t present_queue_index = QUEUE_INDEX_MAX_VALUE) {
    info.physical_device = physical_device;
    info.device = device;
    info.surface = surface;
    info.graphics_queue_index = graphics_queue_index;
    info.present_queue_index = present_queue_index;
    if (graphics_queue_index == QUEUE_INDEX_MAX_VALUE ||
        present_queue_index == QUEUE_INDEX_MAX_VALUE) {
      QueueFamilies queue_families(physical_device.getQueueFamilyProperties());
      if (graphics_queue_index == QUEUE_INDEX_MAX_VALUE)
        info.graphics_queue_index = queue_families.get_graphics_queue_index();
      if (present_queue_index == QUEUE_INDEX_MAX_VALUE)
        info.present_queue_index =
            queue_families.get_present_queue_index(physical_device, surface);
    }
  }
  Swapchain build() const {
    if (!info.surface) {
      throw std::runtime_error("surface_handle_not_provided");
    }

    auto desired_formats = info.desired_formats;
    if (desired_formats.size() == 0)
      add_desired_formats(desired_formats);
    auto desired_present_modes = info.desired_present_modes;
    if (desired_present_modes.size() == 0)
      add_desired_present_modes(desired_present_modes);

    auto surface_support =
        query_surface_support_details(info.physical_device, info.surface);

    uint32_t image_count = surface_support.capabilities.minImageCount + 1;
    if (surface_support.capabilities.maxImageCount > 0 &&
        image_count > surface_support.capabilities.maxImageCount) {
      image_count = surface_support.capabilities.maxImageCount;
    }
    vk::SurfaceFormatKHR surface_format =
        find_surface_format(surface_support.formats, desired_formats);

    vk::Extent2D extent = find_extent(surface_support.capabilities,
                                      info.desired_width, info.desired_height);

    uint32_t image_array_layers = info.array_layer_count;
    if (surface_support.capabilities.maxImageArrayLayers <
        info.array_layer_count)
      image_array_layers = surface_support.capabilities.maxImageArrayLayers;
    if (info.array_layer_count == 0)
      image_array_layers = 1;

    uint32_t queue_family_indices[] = {info.graphics_queue_index,
                                       info.present_queue_index};

    vk::PresentModeKHR present_mode =
        find_present_mode(surface_support.present_modes, desired_present_modes);

    vk::SurfaceTransformFlagBitsKHR pre_transform = info.pre_transform;
    if (info.pre_transform == static_cast<vk::SurfaceTransformFlagBitsKHR>(0))
      pre_transform = surface_support.capabilities.currentTransform;

    vk::SwapchainCreateInfoKHR swapchain_create_info;
    helper::setup_pNext_chain<vk::SwapchainCreateInfoKHR>(swapchain_create_info,
                                                          info.pNext_chain);
    swapchain_create_info.flags = info.create_flags;
    swapchain_create_info.surface = info.surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = image_array_layers;
    swapchain_create_info.imageUsage = info.image_usage_flags;

    if (info.graphics_queue_index != info.present_queue_index) {
      swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
      swapchain_create_info.queueFamilyIndexCount = 2;
      swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
      swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchain_create_info.preTransform = pre_transform;
    swapchain_create_info.compositeAlpha = info.composite_alpha;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = info.clipped;
    swapchain_create_info.oldSwapchain = info.old_swapchain;
    Swapchain swapchain;
    swapchain.instance = info.device.createSwapchainKHR(
        swapchain_create_info, info.allocation_callbacks);

    swapchain.device = info.device;
    swapchain.image_format = surface_format.format;
    swapchain.extent = extent;
    auto images = swapchain.get_images();
    if (images.empty()) {
      throw std::runtime_error("failed_get_swapchain_images");
    }
    swapchain.image_count = static_cast<uint32_t>(images.size());
    swapchain.allocation_callbacks = info.allocation_callbacks;
    return swapchain;
  }

  // Set the oldSwapchain member of VkSwapchainCreateInfoKHR.
  // For use in rebuilding a swapchain.
  SwapchainBuilder &set_old_swapchain(vk::SwapchainKHR old_swapchain) {
    info.old_swapchain = old_swapchain;
    return *this;
  }
  SwapchainBuilder &set_old_swapchain(Swapchain const &swapchain) {
    info.old_swapchain = swapchain;
    return *this;
  }
  // Desired size of the swapchain. By default, the swapchain will use the size
  // of the window being drawn to.
  SwapchainBuilder &set_desired_extent(uint32_t width, uint32_t height) {
    info.desired_width = width;
    info.desired_height = height;
    return *this;
  }
  // When determining the surface format, make this the first to be used if
  // supported.
  SwapchainBuilder &set_desired_format(vk::SurfaceFormatKHR format) {
    info.desired_formats.insert(info.desired_formats.begin(), format);
    return *this;
  }
  // Add this swapchain format to the end of the list of formats selected from.
  SwapchainBuilder &add_fallback_format(vk::SurfaceFormatKHR format) {
    info.desired_formats.push_back(format);
    return *this;
  }
  // Use the default swapchain formats. This is done if no formats are provided.
  // Default surface format is {VK_FORMAT_B8G8R8A8_SRGB,
  // VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
  SwapchainBuilder &use_default_format_selection() {
    info.desired_formats.clear();
    add_desired_formats(info.desired_formats);
    return *this;
  }
  // When determining the present mode, make this the first to be used if
  // supported.
  SwapchainBuilder &set_desired_present_mode(vk::PresentModeKHR present_mode) {
    info.desired_present_modes.insert(info.desired_present_modes.begin(),
                                      present_mode);
    return *this;
  }
  // Add this present mode to the end of the list of present modes selected
  // from.
  SwapchainBuilder &add_fallback_present_mode(vk::PresentModeKHR present_mode) {
    info.desired_present_modes.push_back(present_mode);
    return *this;
  }
  // Use the default presentation mode. This is done if no present modes are
  // provided. Default present modes: VK_PRESENT_MODE_MAILBOX_KHR with fallback
  // VK_PRESENT_MODE_FIFO_KHR
  SwapchainBuilder &use_default_present_mode_selection() {
    info.desired_present_modes.clear();
    add_desired_present_modes(info.desired_present_modes);
    return *this;
  }

  // Set the bitmask of the image usage for acquired swapchain images.
  SwapchainBuilder &set_image_usage_flags(vk::ImageUsageFlags usage_flags) {
    info.image_usage_flags = usage_flags;
    return *this;
  }
  // Add a image usage to the bitmask for acquired swapchain images.
  SwapchainBuilder &add_image_usage_flags(vk::ImageUsageFlags usage_flags) {
    info.image_usage_flags = info.image_usage_flags | usage_flags;
    return *this;
  }
  // Use the default image usage bitmask values. This is the default if no image
  // usages are provided. The default is VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
  SwapchainBuilder &use_default_image_usage_flags() {
    info.image_usage_flags = vk::ImageUsageFlagBits::eColorAttachment;
    return *this;
  }

  // Set the number of views in for multiview/stereo surface
  SwapchainBuilder &set_image_array_layer_count(uint32_t array_layer_count) {
    info.array_layer_count = array_layer_count;
    return *this;
  }

  // Set whether the Vulkan implementation is allowed to discard rendering
  // operations that affect regions of the surface that are not visible. Default
  // is true. Note: Applications should use the default of true if they do not
  // expect to read back the content of presentable images before presenting
  // them or after reacquiring them, and if their fragment shaders do not have
  // any side effects that require them to run for all pixels in the presentable
  // image.
  SwapchainBuilder &set_clipped(bool clipped = true) {
    info.clipped = clipped;
    return *this;
  }

  // Set the VkSwapchainCreateFlagBitsKHR.
  SwapchainBuilder &
  set_create_flags(vk::SwapchainCreateFlagBitsKHR create_flags) {
    info.create_flags = create_flags;
    return *this;
  }
  // Set the transform to be applied, like a 90 degree rotation. Default is no
  // transform.
  SwapchainBuilder &
  set_pre_transform_flags(vk::SurfaceTransformFlagBitsKHR pre_transform_flags) {
    info.pre_transform = pre_transform_flags;
    return *this;
  }
  // Set the alpha channel to be used with other windows in on the system.
  // Default is VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
  SwapchainBuilder &set_composite_alpha_flags(
      vk::CompositeAlphaFlagBitsKHR composite_alpha_flags) {
    info.composite_alpha = composite_alpha_flags;
    return *this;
  }

  // Add a structure to the pNext chain of VkSwapchainCreateInfoKHR.
  // The structure must be valid when SwapchainBuilder::build() is called.
  template <typename T> SwapchainBuilder &add_pNext(T *structure) {
    info.pNext_chain.push_back(
        reinterpret_cast<vk::BaseOutStructure *>(structure));
    return *this;
  }

  // Provide custom allocation callbacks.
  SwapchainBuilder &
  set_allocation_callbacks(vk::Optional<const vk::AllocationCallbacks> callbacks) {
    info.allocation_callbacks = callbacks;
    return *this;
  }

private:
  void add_desired_formats(std::vector<vk::SurfaceFormatKHR> &formats) const {
    formats.push_back(
        {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear});
    formats.push_back(
        {vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear});
  }
  void add_desired_present_modes(std::vector<vk::PresentModeKHR> &modes) const {
    modes.push_back(vk::PresentModeKHR::eMailbox);
    modes.push_back(vk::PresentModeKHR::eFifo);
  }
  struct SwapchainInfo {
    vk::PhysicalDevice physical_device;
    vk::Device device;
    std::vector<vk::BaseOutStructure *> pNext_chain;
    vk::SwapchainCreateFlagBitsKHR create_flags = 
        static_cast<vk::SwapchainCreateFlagBitsKHR>(0);
    vk::SurfaceKHR surface;
    std::vector<vk::SurfaceFormatKHR> desired_formats;
    uint32_t desired_width = 256;
    uint32_t desired_height = 256;
    uint32_t array_layer_count = 1;
    vk::ImageUsageFlags image_usage_flags =
        vk::ImageUsageFlagBits::eColorAttachment;
    uint32_t graphics_queue_index = 0;
    uint32_t present_queue_index = 0;
    vk::SurfaceTransformFlagBitsKHR pre_transform =
        static_cast<vk::SurfaceTransformFlagBitsKHR>(0);
    vk::CompositeAlphaFlagBitsKHR composite_alpha =
        vk::CompositeAlphaFlagBitsKHR::eOpaque;
    std::vector<vk::PresentModeKHR> desired_present_modes;
    bool clipped = true;
    vk::SwapchainKHR old_swapchain;
    vk::Optional<const vk::AllocationCallbacks> allocation_callbacks = nullptr;
  } info;

  vk::PresentModeKHR find_present_mode(
      std::vector<vk::PresentModeKHR> const &available_resent_modes,
      std::vector<vk::PresentModeKHR> const &desired_present_modes) const {
    for (auto const &desired_pm : desired_present_modes) {
      for (auto const &available_pm : available_resent_modes) {
        // finds the first present mode that is desired and available
        if (desired_pm == available_pm)
          return desired_pm;
      }
    }
    // only present mode required, use as a fallback
    return vk::PresentModeKHR::eFifo;
  }

  struct SurfaceSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
  };

  SurfaceSupportDetails
  query_surface_support_details(vk::PhysicalDevice phys_device,
                                vk::SurfaceKHR surface) const {
    if (!surface)
      throw std::runtime_error("surface_handle_null");

    vk::SurfaceCapabilitiesKHR capabilities =
        phys_device.getSurfaceCapabilitiesKHR(surface);

    auto formats = phys_device.getSurfaceFormatsKHR(surface);
    if (formats.empty())
      throw std::runtime_error("failed_enumerate_surface_formats");
    auto present_modes = phys_device.getSurfacePresentModesKHR(surface);
    if (present_modes.empty())
      throw std::runtime_error("failed_enumerate_present_modes");

    return SurfaceSupportDetails{capabilities, formats, present_modes};
  }

  template <typename T> T minimum(T a, T b) const { return a < b ? a : b; }
  template <typename T> T maximum(T a, T b) const { return a > b ? a : b; }

  vk::Extent2D find_extent(vk::SurfaceCapabilitiesKHR const &capabilities,
                           uint32_t desired_width,
                           uint32_t desired_height) const {
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    } else {
      vk::Extent2D actualExtent = {desired_width, desired_height};

      actualExtent.width = maximum(
          capabilities.minImageExtent.width,
          minimum(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = maximum(
          capabilities.minImageExtent.height,
          minimum(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
    }
  }

  vk::SurfaceFormatKHR find_surface_format(
      std::vector<vk::SurfaceFormatKHR> const &available_formats,
      std::vector<vk::SurfaceFormatKHR> const &desired_formats) const {
    for (auto const &desired_format : desired_formats) {
      for (auto const &available_format : available_formats) {
        // finds the first format that is desired and available
        if (desired_format.format == available_format.format &&
            desired_format.colorSpace == available_format.colorSpace) {
          return desired_format;
        }
      }
    }

    // use the first available one if any desired formats aren't found
    return available_formats[0];
  }
};

#pragma endregion

#pragma region RenderPass

class RenderPassBuilder;
class SubpassBuilder {
public:
  SubpassBuilder& addAttachmentRef(int index, vk::ImageLayout layout);
  vk::SubpassDescription build(RenderPassBuilder& rpb, std::vector<vk::AttachmentReference>& refs);
protected:
  std::vector<int> idx;
  std::vector<vk::ImageLayout> layouts;
};

class RenderPassBuilder {
public:
  RenderPassBuilder(Device const &device) 
    : device(device) {
  }
  vk::RenderPass build() {
    vk::RenderPassCreateInfo info;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.subpassCount = subpass.size();
    info.pSubpasses = subpass.data();
    info.dependencyCount = dependencies.size();
    info.pDependencies = dependencies.data();
    return device->createRenderPass(info, device.allocation_callbacks);
  }
   
  RenderPassBuilder& addAttachment(const vk::AttachmentDescription& ad) {
    attachments.push_back(ad);
    return *this;
  }

  RenderPassBuilder& addColorAttachment(vk::Format image_format, 
    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare,
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare
  ) {
    attachments.push_back(
      vk::AttachmentDescription() 
        .setFormat(image_format) 
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(loadOp)
        .setStoreOp(storeOp)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
    );
    return *this;
  }

  RenderPassBuilder& addPresentAttachment(vk::Format image_format, 
    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eLoad,
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore
  ) {
    attachments.push_back(
      vk::AttachmentDescription() 
        .setFormat(image_format) 
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(loadOp)
        .setStoreOp(storeOp)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
    );
    return *this;
  }

  RenderPassBuilder& addSubpass(const vk::SubpassDescription& sd) {
    subpass.push_back(sd);
    return *this;
  }

  RenderPassBuilder& addSubpass(SubpassBuilder& sb) {
    refs.push_back({});
    subpass.push_back(sb.build(*this, refs.back()));
    return *this;
  }

  RenderPassBuilder& addDependency(uint32_t src, uint32_t dst, 
    vk::PipelineStageFlags srcFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput, 
    vk::PipelineStageFlags dstFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput, 
    vk::AccessFlags srcAFlags = {}, 
    vk::AccessFlags dstAFlags = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
    vk::DependencyFlags depFlags = {}
  ) {
    vk::SubpassDependency sd(src, dst, srcFlags, dstFlags, srcAFlags, dstAFlags, depFlags);
    dependencies.push_back(sd);
    return *this;
  }

  friend class SubpassBuilder;
private:
  Device const& device;
  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::SubpassDescription> subpass;
  std::vector<std::vector<vk::AttachmentReference> > refs;
  std::vector<vk::SubpassDependency> dependencies;
};


SubpassBuilder& SubpassBuilder::addAttachmentRef(int index, vk::ImageLayout layout) {
  idx.push_back(index);
  layouts.push_back(layout);
  return *this;
}
vk::SubpassDescription SubpassBuilder::build(RenderPassBuilder& rpb, std::vector<vk::AttachmentReference>& refs) {
  for (int i = 0; i < idx.size(); ++i) {
    vk::AttachmentReference ar;
    ar.attachment = idx[i];
    ar.layout = layouts[i];
    refs.push_back(ar);
  }

  vk::SubpassDescription sd;
  sd.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  sd.colorAttachmentCount = refs.size();
  sd.pColorAttachments = refs.data();
  return sd;
}

#pragma endregion

#pragma region Pipeline


class VertexInputStateBuilder {
private:
  friend class PipelineBuilder;
  vk::PipelineVertexInputStateCreateInfo build() {
    vk::PipelineVertexInputStateCreateInfo info = {};
    info.flags = flags;
    info.vertexAttributeDescriptionCount = static_cast<uint32_t>(input_attributes.size());
    info.pVertexAttributeDescriptions = input_attributes.data();
    info.vertexBindingDescriptionCount = static_cast<uint32_t>(input_bindings.size());
    info.pVertexBindingDescriptions = input_bindings.data();
    return info;
  }
public:
  template<class T>
  VertexInputStateBuilder& addInputBinding() {
    uint32_t binding = input_bindings.size();
    input_bindings.push_back(T::getBindingDescription(binding));
    return *this;
  }

  template<class T>
  VertexInputStateBuilder& addAttributeDescription(int binding = -1) {
    if (binding == -1) binding = input_bindings.size();
    const auto& vec = T::getAttributeDescription(binding-1);
    input_attributes.insert(input_attributes.end(), vec.begin(), vec.end());
    return *this;
  }

  vk::PipelineVertexInputStateCreateFlags flags;
  std::vector< vk::VertexInputBindingDescription > input_bindings;
  std::vector< vk::VertexInputAttributeDescription > input_attributes;
};

class PipelineBuilder {
public:
  PipelineBuilder(const Device &device, const Swapchain & swapchain)
    : device(device), swapchain(swapchain) {
  }

  vk::Pipeline build(vk::RenderPass render_pass, uint32_t subpass = 0) {
    if (use_default_input_state) setVertexInputState(VertexInputStateBuilder().build());
    else if (use_input_state_builder) setVertexInputState(input_state_builder.build());
    
    if (use_default_input_assembly) setInputAssemblyState();
    if (use_default_viewport) addViewport();
    if (use_default_scissor) addScissor();
    if (use_default_rasterizer) setRasterizer();
    if (use_default_multisampling) setMultisampler();
    if (use_default_color_blending) setColorBlending();

    vk::PipelineDynamicStateCreateInfo dynamic_info;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates    = dynamic_states.data();

    vk::PipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.setLayoutCount         = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;
    vk::PipelineLayout layout = device->createPipelineLayout(pipeline_layout_info);

    vk::PipelineViewportStateCreateInfo viewport_state;
    viewport_state.viewportCount = viewports.size();
    viewport_state.pViewports    = viewports.data();
    viewport_state.scissorCount  = scissors.size();
    viewport_state.pScissors     = scissors.data();

    vk::GraphicsPipelineCreateInfo pipeline_info;
    pipeline_info.stageCount          = shader_stages.size();
    pipeline_info.pStages             = shader_stages.data();
    pipeline_info.pVertexInputState   = &input_state;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = &dynamic_info;
    pipeline_info.layout              = layout;
    pipeline_info.renderPass          = render_pass;
    pipeline_info.subpass             = subpass;

    return device->createGraphicsPipeline(nullptr, pipeline_info);
  }

  PipelineBuilder& useClassicPipeline(const std::vector<uint32_t>& vert, const std::vector<uint32_t>& frag) {
    vk::ShaderModule vert_module = createShaderModule(device, vert);
    vk::ShaderModule frag_module = createShaderModule(device, frag);
    return useClassicPipeline(vert_module, frag_module);
  }

  PipelineBuilder& useClassicPipeline(vk::ShaderModule vert_module, vk::ShaderModule frag_module) {
    addVertexStage(vert_module);
    addFragmentStage(frag_module);
    return *this;
  }

  PipelineBuilder& addVertexStage(vk::ShaderModule vert_module, std::string pName = "main") {
    shader_entry_names.push_back(pName);

    vk::PipelineShaderStageCreateInfo stage_info = {};
    stage_info.stage  = vk::ShaderStageFlagBits::eVertex;
    stage_info.module = vert_module;
    stage_info.pName  = shader_entry_names.back().c_str();

    shader_stages.push_back(stage_info);

    return *this;
  }

  PipelineBuilder& addFragmentStage(vk::ShaderModule frag_module, std::string pName = "main") {
    shader_entry_names.push_back(pName);

    vk::PipelineShaderStageCreateInfo stage_info = {};
    stage_info.stage  = vk::ShaderStageFlagBits::eFragment;
    stage_info.module = frag_module;
    stage_info.pName  = shader_entry_names.back().c_str();
    shader_stages.push_back(stage_info);
    return *this;
  }

  PipelineBuilder& addTessellationControlStage(vk::ShaderModule tess_module, std::string pName = "main") {
    shader_entry_names.push_back(pName);

    vk::PipelineShaderStageCreateInfo stage_info = {};
    stage_info.stage  = vk::ShaderStageFlagBits::eTessellationControl;
    stage_info.module = tess_module;
    stage_info.pName  = shader_entry_names.back().c_str();
    shader_stages.push_back(stage_info);
    return *this;
  }

  PipelineBuilder& addTessellationEvaluationStage(vk::ShaderModule tess_module, std::string pName = "main") {
    shader_entry_names.push_back(pName);

    vk::PipelineShaderStageCreateInfo stage_info = {};
    stage_info.stage  = vk::ShaderStageFlagBits::eTessellationEvaluation;
    stage_info.module = tess_module;
    stage_info.pName  = shader_entry_names.back().c_str();
    shader_stages.push_back(stage_info);
    return *this;
  }

  PipelineBuilder& addComputeStage(vk::ShaderModule compute_module, std::string pName = "main") {
    shader_entry_names.push_back(pName);

    vk::PipelineShaderStageCreateInfo stage_info = {};
    stage_info.stage  = vk::ShaderStageFlagBits::eCompute;
    stage_info.module = compute_module;
    stage_info.pName  = shader_entry_names.back().c_str();
    shader_stages.push_back(stage_info);
    return *this;
  }

  PipelineBuilder& setVertexInputState(VertexInputStateBuilder& input_state) {
    use_default_input_state = false;
    use_input_state_builder = true;
    this->input_state_builder = input_state;
    return *this;
  }

  PipelineBuilder& setVertexInputState(vk::PipelineVertexInputStateCreateInfo input_state) {
    use_default_input_state = false;
    use_input_state_builder = false;
    this->input_state = input_state;
    return *this;
  }

  PipelineBuilder& setInputAssemblyState(vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false) {
    use_default_input_assembly = false;
    vk::PipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.topology               = topology;
    input_assembly.primitiveRestartEnable = primitiveRestartEnable;
    this->input_assembly = input_assembly;
    return *this;
  }

  PipelineBuilder& setInputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo input_assembly) {
    use_default_input_assembly = false;
    this->input_assembly = input_assembly;
    return *this;
  }

  PipelineBuilder& addScissor(vk::Rect2D scissor = {}) {
    use_default_scissor = false;
    if (scissor.extent == vk::Extent2D())
      scissor.extent   = swapchain.extent;
    scissors.push_back(scissor);
    return *this;
  }

  PipelineBuilder& addViewport( float width = -1.0f, float height = -1.0f, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f) {
    vk::Viewport viewport = {};
    viewport.x          = x;
    viewport.y          = y;
    viewport.width      = width == -1.0f ? swapchain.extent.width : width;
    viewport.height     = height == -1.0f ? swapchain.extent.height : height;
    viewport.minDepth   = minDepth;
    viewport.maxDepth   = maxDepth;

    addViewport(viewport);
    return *this;
  } 

  PipelineBuilder& addViewport(vk::Viewport viewport) {
    use_default_viewport = false;
    viewports.push_back(viewport);
    return *this;
  }

  PipelineBuilder& setRasterizer(vk::PolygonMode polygonMode = vk::PolygonMode::eFill,
    bool depthClampEnable = false, bool rasterizerDiscardEnable = false, 
    float lineWidth = 1.0f, vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack, 
    vk::FrontFace frontFace = vk::FrontFace::eClockwise, bool depthBiasEnable = false
    ) {
    use_default_rasterizer = false;
    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable        = depthClampEnable;
    rasterizer.rasterizerDiscardEnable = rasterizerDiscardEnable;
    rasterizer.polygonMode             = polygonMode;
    rasterizer.lineWidth               = lineWidth;
    rasterizer.cullMode                = cullMode;
    rasterizer.frontFace               = frontFace;
    rasterizer.depthBiasEnable         = depthBiasEnable;
    this->rasterizer = rasterizer;
    return *this;
  }

  PipelineBuilder& setRasterizer(vk::PipelineRasterizationStateCreateInfo rasterizer) {
    use_default_rasterizer = false;
    this->rasterizer = rasterizer;
    return *this;
  }

  PipelineBuilder& setMultisampler(bool sampleShadingEnable = false, 
    vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1) {
    use_default_multisampling = false;
    multisampling.sampleShadingEnable  = sampleShadingEnable;
    multisampling.rasterizationSamples = rasterizationSamples;
    return *this;
  }


  PipelineBuilder& setMultisampler(vk::PipelineMultisampleStateCreateInfo multisampling) {
    use_default_multisampling = false;
    this->multisampling = multisampling;
    return *this;
  }

  PipelineBuilder& setColorBlending() {
    use_default_color_blending = false;
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = false;

    color_blending.logicOpEnable     = false;
    color_blending.logicOp           = vk::LogicOp::eCopy;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &colorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;
    return *this;
  }

  PipelineBuilder& setColorBlending(vk::PipelineColorBlendStateCreateInfo color_blending) {
    use_default_color_blending = false;
    this->color_blending = color_blending;
    return *this;
  }

  PipelineBuilder& setDynamicStatesViewportScissor() {
    use_default_dynamic_states = false;
    dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    return *this;
  }


  PipelineBuilder& setDynamicStates(std::vector<vk::DynamicState> ds) {
    use_default_dynamic_states = false;
    dynamic_states = ds;
    return *this;
  }

  PipelineBuilder& setDynamicStates(const std::initializer_list<vk::DynamicState>& v) { 
    use_default_dynamic_states = false;
    dynamic_states = v;
    return *this;
  }

  static vk::ShaderModule createShaderModule(const vk::Device& d, const std::vector<uint32_t>& code) {
    vk::ShaderModuleCreateInfo create_info = {};
    create_info.codeSize = code.size() * 4;
    create_info.pCode    = code.data();
    return d.createShaderModule(create_info);
  }

private:
  Device const& device;
  Swapchain const& swapchain;

  std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
  // Cannot use vector, since the string object will deconstruct during pushing back.
  std::deque<std::string> shader_entry_names; 

  bool use_default_dynamic_states = true;
  std::vector<vk::DynamicState> dynamic_states;

  bool use_default_input_state = true;
  vk::PipelineVertexInputStateCreateInfo input_state;
  bool use_input_state_builder = true;
  VertexInputStateBuilder input_state_builder;

  bool use_default_input_assembly = true;
  vk::PipelineInputAssemblyStateCreateInfo input_assembly;

  bool use_default_viewport = true;
  std::vector<vk::Viewport> viewports;

  bool use_default_scissor = true;
  std::vector<vk::Rect2D> scissors;

  bool use_default_rasterizer = true;
  vk::PipelineRasterizationStateCreateInfo rasterizer;

  bool use_default_multisampling = true;
  vk::PipelineMultisampleStateCreateInfo multisampling;

  bool use_default_color_blending = true;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo color_blending;
};


#pragma endregion

#pragma region Present

// Each thread should have a single struct for commands recording
struct Present {
  Swapchain* swapchain = nullptr;
  Device* device = nullptr;

  Present() {}
  Present(Device& device, Swapchain& swapchain)
    : device(&device), swapchain(&swapchain) {}

  vk::Queue  graphics_queue;
  vk::Queue  present_queue;

  vk::CommandPool                command_pool;
  std::vector<vk::CommandBuffer> command_buffers;
  std::vector<vk::Framebuffer>   framebuffers;

  std::vector<vk::Fence> in_flight_fences;
  std::vector<vk::Fence> image_in_flight;

  std::vector<vk::Semaphore> available_semaphores;
  std::vector<vk::Semaphore> finished_semaphore;
  vk::RenderPass render_pass;

  vk::CommandBuffer& getCurrentCommandBuffer() {
    return command_buffers[swapchain->current_frame];
  }

  vk::Framebuffer& getCurrentFrameBuffer() {
    return framebuffers[swapchain->current_frame];
  }

  void begin() {
    vk::CommandBufferBeginInfo begin_info;
    auto buffer = getCurrentCommandBuffer();
    buffer.begin(begin_info);
  }

  void end() {
    auto buffer = getCurrentCommandBuffer();
    buffer.end();
  }

  void beginRenderPass(vk::RenderPass render_pass, 
    vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})) {    
    this->render_pass = render_pass;
    vk::RenderPassBeginInfo render_pass_info = {};
    render_pass_info.renderPass            = render_pass;
    render_pass_info.framebuffer           = getCurrentFrameBuffer();
    render_pass_info.renderArea.offset     = vk::Offset2D{0, 0};
    render_pass_info.renderArea.extent     = swapchain->extent;
    render_pass_info.clearValueCount       = 1;
    render_pass_info.pClearValues          = &clearColor;
    getCurrentCommandBuffer().beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
  }

  void endRenderPass() {
    getCurrentCommandBuffer().endRenderPass();
  }

  vk::Fence& getInFlightFence() {
    return in_flight_fences[swapchain->current_frame];
  }
  vk::Fence& getImageInFlight(uint32_t idx) {
    return image_in_flight[idx];
  }
  vk::Semaphore& getAvailableSemaphore() {
    return available_semaphores[swapchain->current_frame];
  }

  vk::Semaphore& getFinishedSemaphore() {
    return finished_semaphore[swapchain->current_frame];
  }

  void create_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{*device};
    auto swap_ret = swapchain_builder.set_old_swapchain(*swapchain).build();
    swapchain->destroy();
    *swapchain = swap_ret;
  }

  void recreate_swapchain() {
    (*device)->waitIdle();
    (*device)->destroyCommandPool(command_pool, device->allocation_callbacks);
    for (auto framebuffer : framebuffers) {
      (*device)->destroyFramebuffer(framebuffer, device->allocation_callbacks);
    }

    swapchain->destroy_imageviews();
    create_swapchain();
    framebuffers = swapchain->createFramebuffers(this->render_pass);
    command_pool = device->createCommandPool();
    command_buffers = device->createCommandBuffers(
                         command_pool, swapchain->image_count);
  }

  void drawFrame() {
    auto dev = *device;
    dev->waitForFences(1, &getInFlightFence(), true, UINT64_MAX);

    uint32_t image_index = 0;
    vk::Result result = dev->acquireNextImageKHR(*swapchain, UINT64_MAX,
                        getAvailableSemaphore(), vk::Fence(), &image_index);
    if (result == vk::Result::eErrorOutOfDateKHR) {
      recreate_swapchain();
      return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
      std::cout << "failed to acquire swapchain image. Error " << result << "\n";
      return;
    }

    if (getImageInFlight(image_index)) {
        dev->waitForFences(1, &getImageInFlight(image_index), true, UINT64_MAX);
    }
    getImageInFlight(image_index) = getInFlightFence();

    vk::Semaphore          wait_semaphores[] = { getAvailableSemaphore() };
    vk::PipelineStageFlags wait_stages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount          = 1;
    submitInfo.pWaitSemaphores             = wait_semaphores;
    submitInfo.pWaitDstStageMask           = wait_stages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &getCurrentCommandBuffer();
    vk::Semaphore signal_semaphores[] = {getFinishedSemaphore()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signal_semaphores;

    dev->resetFences(1, &getInFlightFence());
    graphics_queue.submit(1, &submitInfo, getInFlightFence());

    dev->waitIdle();

    vk::PresentInfoKHR present_info = {};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    vk::SwapchainKHR swapChains[] = {swapchain->instance};
    present_info.swapchainCount   = 1;
    present_info.pSwapchains      = swapChains;
    present_info.pImageIndices    = &image_index;

    result = present_queue.presentKHR(&present_info);
    if (result == vk::Result::eErrorOutOfDateKHR 
         || result == vk::Result::eSuboptimalKHR) {
      recreate_swapchain();
    } else if (result != vk::Result::eSuccess) {
      std::cout << "failed to present swapchain image\n";
      return;
    }
    swapchain->current_frame = (swapchain->current_frame + 1) % swapchain->image_count;
  }
};


class PresentBuilder {
public:
  PresentBuilder(Device& device, Swapchain& swapchain) 
    : device(device), swapchain(swapchain) {}
  virtual ~PresentBuilder() {}

  Present build(vk::RenderPass render_pass) {
    Present cb{device, swapchain};
    cb.command_pool = device.createCommandPool();
    cb.command_buffers = device.createCommandBuffers(
                         cb.command_pool, swapchain.image_count);
    cb.framebuffers = swapchain.createFramebuffers(render_pass);

    cb.in_flight_fences = device.createFences(swapchain.image_count);
    cb.image_in_flight = device.createFences(swapchain.image_count);
    cb.available_semaphores = device.createSemaphores(swapchain.image_count);
    cb.finished_semaphore = device.createSemaphores(swapchain.image_count);

    cb.graphics_queue = device.getQueue(QueueType::graphics);
    cb.present_queue = device.getQueue(QueueType::present);
    return cb;
  }

protected:
  Device& device;
  Swapchain& swapchain;
};


#pragma endregion 


#pragma region Buffer


/// A generic buffer that may be used as a vertex buffer, uniform buffer or other kinds of memory resident data.
/// Buffers require memory objects which represent GPU and CPU resources.
class GenericBuffer {
public:
  GenericBuffer() {
  }

  GenericBuffer(vk::Device device, const vk::PhysicalDeviceMemoryProperties &memprops, vk::BufferUsageFlags usage, vk::DeviceSize size, vk::MemoryPropertyFlags memflags = vk::MemoryPropertyFlagBits::eDeviceLocal) {
    // Create the buffer object without memory.
    vk::BufferCreateInfo ci{};
    ci.size = size_ = size;
    ci.usage = usage;
    ci.sharingMode = vk::SharingMode::eExclusive;
    buffer_ = device.createBufferUnique(ci);

    // Find out how much memory and which heap to allocate from.
    auto memreq = device.getBufferMemoryRequirements(*buffer_);

    // Create a memory object to bind to the buffer.
    vk::MemoryAllocateInfo mai{};
    mai.allocationSize = memreq.size;
    mai.memoryTypeIndex = findMemoryTypeIndex(memprops, memreq.memoryTypeBits, memflags);
    mem_ = device.allocateMemoryUnique(mai);

    device.bindBufferMemory(*buffer_, *mem_, 0);
  }

  /// For a host visible buffer, copy memory to the buffer object.
  void updateLocal(const vk::Device &device, const void *value, vk::DeviceSize size) const {
    void *ptr = device.mapMemory(*mem_, 0, size_, vk::MemoryMapFlags{});
    memcpy(ptr, value, (size_t)size);
    flush(device);
    device.unmapMemory(*mem_);
  }

  inline static /// Utility function for finding memory types for uniforms and images.
  int findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties &memprops, uint32_t memoryTypeBits, vk::MemoryPropertyFlags search) {
    for (int i = 0; i != memprops.memoryTypeCount; ++i, memoryTypeBits >>= 1) {
      if (memoryTypeBits & 1) {
        if ((memprops.memoryTypes[i].propertyFlags & search) == search) {
          return i;
        }
      }
    }
    return -1;
  }

  inline static /// Execute commands immediately and wait for the device to finish.
  void executeImmediately(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, const std::function<void (vk::CommandBuffer cb)> &func) {
    vk::CommandBufferAllocateInfo cbai{ commandPool, vk::CommandBufferLevel::ePrimary, 1 };

    auto cbs = device.allocateCommandBuffers(cbai);
    cbs[0].begin(vk::CommandBufferBeginInfo{});
    func(cbs[0]);
    cbs[0].end();

    vk::SubmitInfo submit;
    submit.commandBufferCount = (uint32_t)cbs.size();
    submit.pCommandBuffers = cbs.data();
    queue.submit(submit, vk::Fence{});
    device.waitIdle();

    device.freeCommandBuffers(commandPool, cbs);
  }

  /// For a purely device local buffer, copy memory to the buffer object immediately.
  /// Note that this will stall the pipeline!
  void upload(vk::Device device, const vk::PhysicalDeviceMemoryProperties &memprops, vk::CommandPool commandPool, vk::Queue queue, const void *value, vk::DeviceSize size) const {
    if (size == 0) return;
    using buf = vk::BufferUsageFlagBits;
    using pfb = vk::MemoryPropertyFlagBits;
    auto tmp = GenericBuffer(device, memprops, buf::eTransferSrc, size, pfb::eHostVisible);
    tmp.updateLocal(device, value, size);

    executeImmediately(device, commandPool, queue, [&](vk::CommandBuffer cb) {
      vk::BufferCopy bc{0, 0, size};
      cb.copyBuffer(tmp.buffer(), *buffer_, bc);
    });
  }

  template<typename T>
  void upload(vk::Device device, const vk::PhysicalDeviceMemoryProperties &memprops, vk::CommandPool commandPool, vk::Queue queue, const std::vector<T> &value) const {
    upload(device, memprops, commandPool, queue, value.data(), value.size() * sizeof(T));
  }

  template<typename T>
  void upload(vk::Device device, const vk::PhysicalDeviceMemoryProperties &memprops, vk::CommandPool commandPool, vk::Queue queue, const T &value) const {
    upload(device, memprops, commandPool, queue, &value, sizeof(value));
  }

  void barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex) const {
    vk::BufferMemoryBarrier bmb{srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, *buffer_, 0, size_};
    cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, nullptr, bmb, nullptr);
  }

  template<class Type, class Allocator>
  void updateLocal(const vk::Device &device, const std::vector<Type, Allocator> &value) const {
    updateLocal(device, (void*)value.data(), vk::DeviceSize(value.size() * sizeof(Type)));
  }

  template<class Type>
  void updateLocal(const vk::Device &device, const Type &value) const {
    updateLocal(device, (void*)&value, vk::DeviceSize(sizeof(Type)));
  }

  void *map(const vk::Device &device) const { return device.mapMemory(*mem_, 0, size_, vk::MemoryMapFlags{}); };
  void unmap(const vk::Device &device) const { return device.unmapMemory(*mem_); };

  void flush(const vk::Device &device) const {
    vk::MappedMemoryRange mr{*mem_, 0, VK_WHOLE_SIZE};
    return device.flushMappedMemoryRanges(mr);
  }

  void invalidate(const vk::Device &device) const {
    vk::MappedMemoryRange mr{*mem_, 0, VK_WHOLE_SIZE};
    return device.invalidateMappedMemoryRanges(mr);
  }

  vk::Buffer buffer() const { return *buffer_; }
  vk::DeviceMemory mem() const { return *mem_; }
  vk::DeviceSize size() const { return size_; }
private:
  vk::UniqueBuffer buffer_;
  vk::UniqueDeviceMemory mem_;
  vk::DeviceSize size_;
};

/// This class is a specialisation of GenericBuffer for high performance vertex buffers on the GPU.
/// You must upload the contents before use.
class VertexBuffer : public GenericBuffer {
public:
  VertexBuffer() {
  }

  VertexBuffer(const vk::Device &device, const vk::PhysicalDeviceMemoryProperties &memprops, size_t size) : GenericBuffer(device, memprops, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, size, vk::MemoryPropertyFlagBits::eDeviceLocal) {
  }
};

/// This class is a specialisation of GenericBuffer for low performance vertex buffers on the host.
class HostVertexBuffer : public GenericBuffer {
public:
  HostVertexBuffer() {
  }

  template<class Type, class Allocator>
  HostVertexBuffer(const vk::Device &device, const vk::PhysicalDeviceMemoryProperties &memprops, const std::vector<Type, Allocator> &value) : GenericBuffer(device, memprops, vk::BufferUsageFlagBits::eVertexBuffer, value.size() * sizeof(Type), vk::MemoryPropertyFlagBits::eHostVisible) {
    updateLocal(device, value);
  }
};

/// This class is a specialisation of GenericBuffer for high performance index buffers.
/// You must upload the contents before use.
class IndexBuffer : public GenericBuffer {
public:
  IndexBuffer() {
  }

  IndexBuffer(const vk::Device &device, const vk::PhysicalDeviceMemoryProperties &memprops, vk::DeviceSize size) : GenericBuffer(device, memprops, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, size, vk::MemoryPropertyFlagBits::eDeviceLocal) {
  }
};

/// This class is a specialisation of GenericBuffer for low performance vertex buffers in CPU memory.
class HostIndexBuffer : public GenericBuffer {
public:
  HostIndexBuffer() {
  }

  template<class Type, class Allocator>
  HostIndexBuffer(const vk::Device &device, const vk::PhysicalDeviceMemoryProperties &memprops, const std::vector<Type, Allocator> &value) : GenericBuffer(device, memprops, vk::BufferUsageFlagBits::eIndexBuffer, value.size() * sizeof(Type), vk::MemoryPropertyFlagBits::eHostVisible) {
    updateLocal(device, value);
  }
};

/// This class is a specialisation of GenericBuffer for uniform buffers.
class UniformBuffer : public GenericBuffer {
public:
  UniformBuffer() {
  }

  /// Device local uniform buffer.
  UniformBuffer(const vk::Device &device, const vk::PhysicalDeviceMemoryProperties &memprops, size_t size) : GenericBuffer(device, memprops, vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst, (vk::DeviceSize)size, vk::MemoryPropertyFlagBits::eDeviceLocal) {
  }
};


#pragma endregion



} // namespace vkb


#if defined(VKB_IMPL)
namespace vkb {

bool InstanceBuilder::loaded = false;

namespace helper { 
VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  auto ms = to_string_message_severity(messageSeverity);
  auto mt = to_string_message_type(messageType);
  printf("[%s: %s]\n%s\n", ms, mt, pCallbackData->pMessage);
  return VK_FALSE;
}
}
}
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif
