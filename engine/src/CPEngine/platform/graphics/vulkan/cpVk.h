#pragma once

#ifdef _WIN32
#define _AMD64_
#include <windef.h>
#define LIBRARY_TYPE HMODULE
#elif defined __linux
#define LIBRARY_TYPE void *
#endif

#include <vector>
#include <vulkan/vulkan.h>

namespace cp::graphics::vulkan {
struct QueueInfo {
  uint32_t familyIndex;
  std::vector<float> priorities;
};
struct PresentInfo {
  VkSwapchainKHR swapchain;
  uint32_t imageIndex;
};

struct ImageTransition {
  VkImage image;
  VkAccessFlags currentAccess;
  VkAccessFlags newAccess;
  VkImageLayout currentLayout;
  VkImageLayout newLayout;
  uint32_t currentQueueFamily;
  uint32_t newQueueFamily;
  VkImageAspectFlags aspect;
};

struct WaitSemaphoreInfo {
  VkSemaphore semaphore;
  VkPipelineStageFlags waitingStage;
};

extern LIBRARY_TYPE VULKAN_LIBRARY;
extern VkDebugUtilsMessengerEXT DEBUG_CALLBACK;

// functions
bool loadFunctionExportedFromVulkanLoaderLibrary(
    LIBRARY_TYPE const &vulkanLibrary);
bool loadGlobalLevelFunctions();

bool checkAvailableInstanceExtensions(
    std::vector<VkExtensionProperties> &availableExtensions);

bool createVulkanInstance(std::vector<char const *> const &desiredExtensions,
                          char const *const applicationName,
                          VkInstance &instance);
bool registerDebugCallback(VkInstance instance);

bool createVulkanInstanceWithWsiExtensionsEnabled(
    std::vector<char const *> &desiredExtensions,
    char const *const applicationName, VkInstance &instance);
bool loadInstanceLevelFunctions(
    VkInstance instance, std::vector<char const *> const &enabledExtensions);

bool enumerateAvailablePhysicalDevices(
    const VkInstance instance, std::vector<VkPhysicalDevice> &availableDevices);

bool selectIndexOfQueueFamilyWithDesiredCapabilities(
    const VkPhysicalDevice physicalDevice,
    const VkQueueFlags desiredCapabilities, uint32_t &queueFamilyIndex);

bool selectQueueFamilyThatSupportsPresentationToGivenSurface(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR presentationSurface,
    uint32_t &queueFamilyIndex);

bool createLogicalDeviceWithWsiExtensionsEnabled(
    VkPhysicalDevice physicalDevice, std::vector<QueueInfo> queueInfos,
    std::vector<char const *> &desiredExtensions,
    VkPhysicalDeviceFeatures2 *desiredFeatures, VkDevice &logicalDevice);

bool loadDeviceLevelFunctions(
    VkDevice logicalDevice, std::vector<char const *> const &enabledExtensions);

} // namespace cp::graphics::vulkan
