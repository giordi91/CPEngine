#include "CPEngine/platform/graphics/vulkan/cpVk.h"
#include "CPEngine/core/core.h"
#include "CPEngine/core/logging.h"
#include "CPEngine/platform/graphics/vulkan/vulkanFunctions.h"
#include <iostream>
#include <vector>

namespace cp::graphics::vulkan {

LIBRARY_TYPE VULKAN_LIBRARY = nullptr;
VkDebugUtilsMessengerEXT DEBUG_CALLBACK = nullptr;

bool loadFunctionExportedFromVulkanLoaderLibrary(
    LIBRARY_TYPE const &vulkanLibrary) {
#if defined _WIN32
#define LoadFunction GetProcAddress
#elif defined __linux
#define LoadFunction dlsym
#endif

#define EXPORTED_VULKAN_FUNCTION(name)                                         \
  name = (PFN_##name)LoadFunction(vulkanLibrary, #name);                       \
  if (name == nullptr) {                                                       \
    logCoreError(core::STRING_POOL->concatenateFrame(                          \
        "Could not load exported Vulkan function named: ", #name, ""));        \
    return false;                                                              \
  }

#include "CPEngine/platform/graphics/vulkan/listOfVulkanFunctions.inl"

  return true;
} // namespace cp::graphics::vulkan

bool loadGlobalLevelFunctions() {
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name);                    \
  if (name == nullptr) {                                                       \
    logCoreError(core::STRING_POOL->concatenateFrame(                          \
        "Could not load exported Vulkan function named: ", #name, ""));        \
    return false;                                                              \
  }

#include "CPEngine/platform/graphics/vulkan/listOfVulkanFunctions.inl"

  return true;
}
bool isExtensionSupported(
    std::vector<VkExtensionProperties> const &availableExtensions,
    char const *const extension) {
  for (auto &availableExtension : availableExtensions) {
    if (strstr(availableExtension.extensionName, extension)) {
      return true;
    }
  }
  return false;
}

bool checkAvailableInstanceExtensions(
    std::vector<VkExtensionProperties> &availableExtensions) {
  uint32_t extensionsCount = 0;
  VkResult result = VK_SUCCESS;

  result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                  nullptr);
  if ((result != VK_SUCCESS) || (extensionsCount == 0)) {
    logCoreError("Could not get the number of instance extensions.");
    return false;
  }

  availableExtensions.resize(extensionsCount);
  result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                  availableExtensions.data());
  if ((result != VK_SUCCESS) || (extensionsCount == 0)) {
    logCoreError("Could not enumerate instance extensions.");
    return false;
  }

  return true;
}

bool createVulkanInstance(std::vector<char const *> const &desiredExtensions,
                          char const *const applicationName,
                          VkInstance &instance) {
  std::vector<VkExtensionProperties> availableExtensions;
  if (!checkAvailableInstanceExtensions(availableExtensions)) {
    return false;
  }

  for (auto &extension : desiredExtensions) {
    if (!isExtensionSupported(availableExtensions, extension)) {
      logCoreError(core::STRING_POOL->concatenateFrame(
          "Extension named '", "' is not supported by an Instance object.",
          extension));
      return false;
    }
  }

  VkApplicationInfo applicationInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                       nullptr,
                                       applicationName,
                                       VK_MAKE_VERSION(1, 0, 0),
                                       "Vulkan Cookbook",
                                       VK_MAKE_VERSION(1, 0, 0),
                                       VK_API_VERSION_1_1};

  VkInstanceCreateInfo instanceCreateInfo = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      nullptr,
      0,
      &applicationInfo,
      0,
      nullptr,
      static_cast<uint32_t>(desiredExtensions.size()),
      desiredExtensions.data()};

  // TODO: change this to flgas possibly from the context creation
#if _DEBUG
  const char *layers[] = {
    "VK_LAYER_LUNARG_standard_validation",

#if VULKAN_OBJ_TRACKER
    "VK_LAYER_LUNARG_object_tracker",
#endif
#if VULKAN_PARAM_VALIDATION
    "VK_LAYER_LUNARG_parameter_validation",
#endif
  };
  instanceCreateInfo.ppEnabledLayerNames = layers;
  instanceCreateInfo.enabledLayerCount = ARRAYSIZE(layers);
#endif

  const VkResult result =
      vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if ((result != VK_SUCCESS) || (instance == VK_NULL_HANDLE)) {
    logCoreError("Could not create Vulkan instance.");
    return false;
  }

  return true;
}

// proper function signature is
// debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//              VkDebugUtilsMessageTypeFlagsEXT messageType,
//              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
//              void *pUserData) {
// hidden names to avoid unused warnings

VkBool32 VKAPI_PTR debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {

  const char *type =
      messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
          ? "ERROR"
          : (messageSeverity &
             (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
                ? "WARNING"
                : "INFO";

  char message[4096];
  snprintf(message, ARRAYSIZE(message), "%s: %s\n", type,
           pCallbackData->pMessage);
#if _WIN32
  OutputDebugStringA(message);
#endif

  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    logCoreError(message);
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    logCoreWarn(message);
  } else {
    logCoreInfo(message);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    assert(0 && "validation layer assertion");
  }

  // always need to return false, true is reserved for layer development
  return VK_FALSE;
}

bool registerDebugCallback(VkInstance instance) {

  // create debug utils messenger
  VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {};
  debug_utils_messenger_create_info.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_utils_messenger_create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
  debug_utils_messenger_create_info.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debug_utils_messenger_create_info.pfnUserCallback = debugCallback;
  if (VK_SUCCESS !=
      vkCreateDebugUtilsMessengerEXT(
          instance, &debug_utils_messenger_create_info, NULL, &DEBUG_CALLBACK))
    exit(EXIT_FAILURE);
  return true;
}

bool createVulkanInstanceWithWsiExtensionsEnabled(
    std::vector<char const *> &desiredExtensions,
    char const *const applicationName, VkInstance &instance) {
  desiredExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
  desiredExtensions.emplace_back(
#ifdef VK_USE_PLATFORM_WIN32_KHR
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XCB_KHR
      VK_KHR_XCB_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XLIB_KHR
      VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
  );
  // used for validation layer error DEBUG_CALLBACK
  desiredExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  desiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return createVulkanInstance(desiredExtensions, applicationName, instance);
}

bool loadInstanceLevelFunctions(VkInstance instance,
                                std::vector<char const *> const &extensions) {
  // Load core Vulkan API instance-level functions
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name)                                   \
  name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);                   \
  if (name == nullptr) {                                                       \
    logCoreError(core::STRING_POOL->concatenateFrame(                          \
        "Could not load exported Vulkan function named: ", #name, ""));        \
    return false;                                                              \
  }

  // Load instance-level functions from enabled extensions
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  for (auto &enabled_extension : extensions) {                                 \
    if (std::string(enabled_extension) == std::string(extension)) {            \
      name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);               \
      if (name == nullptr) {                                                   \
        logCoreError(core::STRING_POOL->concatenateFrame(                      \
            "Could not load exported Vulkan function named: ", #name, ""));    \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
  }

#include "CPEngine/platform/graphics/vulkan/listOfVulkanFunctions.inl"

  return true;
}

bool loadDeviceLevelFunctions(
    const VkDevice logicalDevice,
    std::vector<char const *> const &enabledExtensions) {
  // Load core Vulkan API device-level functions
#define DEVICE_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetDeviceProcAddr(logicalDevice, #name);                \
  if (name == nullptr) {                                                       \
    std::cout << "Could not load device-level Vulkan function named: " #name   \
              << std::endl;                                                    \
    return false;                                                              \
  }

  // Load device-level functions from enabled extensions
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  for (auto &enabledExtension : enabledExtensions) {                           \
    if (std::string(enabledExtension) == std::string(extension)) {             \
      name = (PFN_##name)vkGetDeviceProcAddr(logicalDevice, #name);            \
      if (name == nullptr) {                                                   \
        std::cout                                                              \
            << "Could not load device-level Vulkan function named: " #name     \
            << std::endl;                                                      \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
  }

#include "CPEngine/platform/graphics/vulkan/listOfVulkanFunctions.inl"

  return true;
}

bool enumerateAvailablePhysicalDevices(
    const VkInstance instance,
    std::vector<VkPhysicalDevice> &availableDevices) {

  uint32_t devicesCount = 0;
  VkResult result = VK_SUCCESS;

  // first time you pass a null pointer so you can ask the size, then resize the
  // memory and finally you can query
  result = vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr);
  if ((result != VK_SUCCESS) || (devicesCount == 0)) {
    logCoreError("Could not get the number of available physical devices.");
    return false;
  }

  availableDevices.resize(devicesCount);
  result = vkEnumeratePhysicalDevices(instance, &devicesCount,
                                      availableDevices.data());
  if ((result != VK_SUCCESS) || (devicesCount == 0)) {
    logCoreError("Could not enumerate physical devices.");
    return false;
  }

  VkPhysicalDeviceProperties properties;
  logCoreInfo("Available devices:");
  for (int i = 0; i < availableDevices.size(); ++i) {
    vkGetPhysicalDeviceProperties(availableDevices[i], &properties);
    logCoreInfo(properties.deviceName);
  }

  return true;
}
void getFeaturesAndPropertiesOfPhysicalDevice(
    const VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceFeatures2 &deviceFeatures,
    VkPhysicalDeviceProperties &deviceProperties) {
  vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
}

bool checkAvailableQueueFamiliesAndTheirProperties(
    const VkPhysicalDevice physicalDevice,
    std::vector<VkQueueFamilyProperties> &queueFamilies) {
  uint32_t queueFamiliesCount = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
                                           nullptr);
  if (queueFamiliesCount == 0) {
    logCoreError("Could not get the number of queue families.");
    return false;
  }

  queueFamilies.resize(queueFamiliesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
                                           queueFamilies.data());
  if (queueFamiliesCount == 0) {
    logCoreError("Could not acquire properties of queue families.");
    return false;
  }

  return true;
}

bool selectIndexOfQueueFamilyWithDesiredCapabilities(
    const VkPhysicalDevice physicalDevice,
    const VkQueueFlags desiredCapabilities, uint32_t &queueFamilyIndex) {
  std::vector<VkQueueFamilyProperties> queueFamilies;
  if (!checkAvailableQueueFamiliesAndTheirProperties(physicalDevice,
                                                     queueFamilies)) {
    return false;
  }

  for (uint32_t index = 0; index < static_cast<uint32_t>(queueFamilies.size());
       ++index) {
    if ((queueFamilies[index].queueCount > 0) &&
        ((queueFamilies[index].queueFlags & desiredCapabilities) ==
         desiredCapabilities)) {
      queueFamilyIndex = index;
      return true;
    }
  }
  return false;
}

bool checkAvailableDeviceExtensions(
    const VkPhysicalDevice physicalDevice,
    std::vector<VkExtensionProperties> &availableExtensions) {
  uint32_t extensionsCount = 0;
  VkResult result = VK_SUCCESS;

  result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                                &extensionsCount, nullptr);
  if ((result != VK_SUCCESS) || (extensionsCount == 0)) {
    logCoreError("Could not get the number of device extensions.");
    return false;
  }

  availableExtensions.resize(extensionsCount);
  result = vkEnumerateDeviceExtensionProperties(
      physicalDevice, nullptr, &extensionsCount, availableExtensions.data());
  if ((result != VK_SUCCESS) || (extensionsCount == 0)) {
    logCoreError("Could not enumerate device extensions.");
    return false;
  }

  return true;
}

bool createLogicalDevice(const VkPhysicalDevice physicalDevice,
                         std::vector<QueueInfo> queueInfos,
                         std::vector<char const *> const &desiredExtensions,
                         VkPhysicalDeviceFeatures2 *desiredFeatures,
                         VkDevice &logicalDevice) {
  std::vector<VkExtensionProperties> availableExtensions;
  if (!checkAvailableDeviceExtensions(physicalDevice, availableExtensions)) {
    return false;
  }

  for (auto &extension : desiredExtensions) {
    if (!isExtensionSupported(availableExtensions, extension)) {
      logCoreError(core::STRING_POOL->concatenateFrame(
          "Extension named '", "' is not supported by a physical device.",
          extension));
      return false;
    }
  }

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  queueCreateInfos.reserve(queueInfos.size());
  for (auto &info : queueInfos) {
    queueCreateInfos.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                nullptr, 0, info.familyIndex,
                                static_cast<uint32_t>(info.priorities.size()),
                                info.priorities.data()});
  };

  VkDeviceCreateInfo deviceCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      desiredFeatures,
      0,
      static_cast<uint32_t>(queueCreateInfos.size()),
      queueCreateInfos.data(),
      0,
      nullptr,
      static_cast<uint32_t>(desiredExtensions.size()),
      desiredExtensions.data()};

  const VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo,
                                         nullptr, &logicalDevice);
  if ((result != VK_SUCCESS) || (logicalDevice == VK_NULL_HANDLE)) {
    logCoreError("Could not create logical device.");
    return false;
  }
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

  logCoreInfo(core::STRING_POOL->concatenateFrame("Physical device used:\n",
                                                  properties.deviceName));

  // TODO add number conversion to the string pool
  char x[64];
  sprintf(x, "%i", properties.driverVersion);
  logCoreInfo(core::STRING_POOL->concatenateFrame("Driver version: ", x, ""));

  logCoreInfo("VRAM: ");
  for (unsigned int i = 0; i < memoryProperties.memoryHeapCount; ++i) {
    sprintf(x, "%f",
            static_cast<float>(memoryProperties.memoryHeaps[i].size * 1.0e-9));
    logCoreInfo(
        core::STRING_POOL->concatenateFrame("    Heap size:", " GB", x));
    logCoreInfo("    Heap type:");
    for (unsigned int h = 0; h < memoryProperties.memoryTypeCount; ++h) {
      if (memoryProperties.memoryTypes[h].heapIndex == i) {
        if (memoryProperties.memoryTypes[h].propertyFlags &
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
          logCoreInfo("        DEVICE_LOCAL");
        }
        if (memoryProperties.memoryTypes[h].propertyFlags &
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
          logCoreInfo("        HOST_VISIBLE");
        }
        if (memoryProperties.memoryTypes[h].propertyFlags &
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
          logCoreInfo("        HOST_COHERENT");
        }
        if (memoryProperties.memoryTypes[h].propertyFlags &
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
          logCoreInfo("        HOST_CACHED");
        }
        if (memoryProperties.memoryTypes[h].propertyFlags &
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
          logCoreInfo("        LAZY_ALLOCATED");
        }
        if (memoryProperties.memoryTypes[h].propertyFlags &
            VK_MEMORY_PROPERTY_PROTECTED_BIT) {
          logCoreInfo("        PROTECTED_BIT");
        }
      }
    }
  }

  return true;
}

bool createLogicalDeviceWithWsiExtensionsEnabled(
    const VkPhysicalDevice physicalDevice,
    const std::vector<QueueInfo> queueInfos,
    std::vector<char const *> &desiredExtensions,
    VkPhysicalDeviceFeatures2 *desiredFeatures, VkDevice &logicalDevice) {
  desiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  desiredExtensions.emplace_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
  desiredExtensions.emplace_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
  desiredExtensions.emplace_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);

  return createLogicalDevice(physicalDevice, queueInfos, desiredExtensions,
                             desiredFeatures, logicalDevice);
}
bool selectQueueFamilyThatSupportsPresentationToGivenSurface(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR presentationSurface,
    uint32_t &queueFamilyIndex) {
  std::vector<VkQueueFamilyProperties> queue_families;
  if (!checkAvailableQueueFamiliesAndTheirProperties(physicalDevice,
                                                     queue_families)) {
    return false;
  }

  for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size());
       ++index) {
    VkBool32 presentation_supported = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
        physicalDevice, index, presentationSurface, &presentation_supported);
    if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
      queueFamilyIndex = index;
      return true;
    }
  }
  return false;
}

void getDeviceQueue(const VkDevice logicalDevice,
                    const uint32_t queueFamilyIndex, const uint32_t queueIndex,
                    VkQueue &queue) {
  vkGetDeviceQueue(logicalDevice, queueFamilyIndex, queueIndex, &queue);
}

bool waitForAllSubmittedCommandsToBeFinished(const VkDevice logicalDevice) {
  const VkResult result = vkDeviceWaitIdle(logicalDevice);
  if (VK_SUCCESS != result) {
    std::cout << "Waiting on a device failed." << std::endl;
    return false;
  }
  return true;
}

bool newSemaphore(const VkDevice logicalDevice, VkSemaphore &semaphore) {
  VkSemaphoreCreateInfo semaphoreCreateInfo = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};

  const VkResult result = vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo,
                                            nullptr, &semaphore);
  if (VK_SUCCESS != result) {
    std::cout << "Could not create a semaphore." << std::endl;
    return false;
  }
  return true;
}

bool presentImage(VkQueue queue, std::vector<VkSemaphore> renderingSemaphores,
                  std::vector<PresentInfo> imagesToPresent) {
  std::vector<VkSwapchainKHR> swapchains;
  std::vector<uint32_t> imageIndices;

  for (auto &imageToPresent : imagesToPresent) {
    swapchains.emplace_back(imageToPresent.swapchain);
    imageIndices.emplace_back(imageToPresent.imageIndex);
  }

  VkPresentInfoKHR presentInfo = {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      nullptr,
      static_cast<uint32_t>(renderingSemaphores.size()),
      renderingSemaphores.data(),
      static_cast<uint32_t>(swapchains.size()),
      swapchains.data(),
      imageIndices.data(),
      nullptr};

  const VkResult result = vkQueuePresentKHR(queue, &presentInfo);
  switch (result) {
  case VK_SUCCESS:
    return true;
  default:
    return false;
  }
}
VkRenderPass createRenderPass(VkDevice logicalDevice, VkFormat format) {
  VkRenderPass renderPass{};

  VkAttachmentDescription attachments[1] = {};
  attachments[0].format = format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // the index here, 0, refers to the index in the attachment array;
  VkAttachmentReference attachReference{
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkSubpassDescription subPass{};
  subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subPass.colorAttachmentCount = 1;
  subPass.pColorAttachments = &attachReference;

  VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = attachments;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subPass;

  vkCreateRenderPass(logicalDevice, &createInfo, nullptr, &renderPass);
  return renderPass;
}

VkFramebuffer createFrameBuffer(const VkDevice logicalDevice,
                                const VkRenderPass renderPass,
                                VkImageView imageView, const uint32_t width,
                                const uint32_t height) {
  VkFramebufferCreateInfo createInfo = {
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  createInfo.renderPass = renderPass;
  createInfo.pAttachments = &imageView;
  createInfo.attachmentCount = 1;
  createInfo.width = width;
  createInfo.height = height;
  createInfo.layers = 1;

  VkFramebuffer frameBuffer = nullptr;
  vkCreateFramebuffer(logicalDevice, &createInfo, nullptr, &frameBuffer);
  return frameBuffer;
}
  bool createCommandPool(const VkDevice logicalDevice,
                       const VkCommandPoolCreateFlags parameters,
                       const uint32_t queueFamily, VkCommandPool &commandPool) {
  VkCommandPoolCreateInfo commandPoolCreateInfo = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, parameters,
      queueFamily};

  const VkResult result = vkCreateCommandPool(
      logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool);
  if (VK_SUCCESS != result) {
    std::cout << "Could not create command pool." << std::endl;
    return false;
  }
  return true;
}
bool allocateCommandBuffers(const VkDevice logicalDevice,
                            const VkCommandPool commandPool,
                            const VkCommandBufferLevel level,
                            const uint32_t count,
                            std::vector<VkCommandBuffer> &commandBuffers) {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, commandPool,
      level, count};

  commandBuffers.resize(count);

  const VkResult result = vkAllocateCommandBuffers(
      logicalDevice, &commandBufferAllocateInfo, commandBuffers.data());
  if (VK_SUCCESS != result) {
    std::cout << "Could not allocate command buffers." << std::endl;
    return false;
  }
  return true;
}

} // namespace cp::graphics::vulkan