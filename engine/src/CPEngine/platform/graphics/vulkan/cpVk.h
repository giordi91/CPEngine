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

#define VK_CHECK(call)                                                         \
  do {                                                                         \
    VkResult result_ = call;                                                   \
    assert(result_ == VK_SUCCESS);                                             \
  } while (0)

#define SET_DEBUG_NAME(resource, type, name)                                   \
  {                                                                            \
    VkDebugUtilsObjectNameInfoEXT debugInfo_{};                                \
    debugInfo_.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;     \
    debugInfo_.objectHandle = (uint64_t)resource;                              \
    debugInfo_.objectType = type;                                              \
    debugInfo_.pObjectName = name;                                             \
    vkSetDebugUtilsObjectNameEXT(LOGICAL_DEVICE, &debugInfo_);                 \
  }

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
bool registerDebugCallback2(VkInstance instance); 

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

void getDeviceQueue(const VkDevice logicalDevice,
                    const uint32_t queueFamilyIndex, const uint32_t queueIndex,
                    VkQueue &queue);

bool newSemaphore(const VkDevice logicalDevice, VkSemaphore &semaphore);
bool waitForAllSubmittedCommandsToBeFinished(const VkDevice logicalDevice);

VkRenderPass createRenderPass(VkDevice logicalDevice,VkFormat format);
VkFramebuffer createFrameBuffer(VkDevice logicalDevice, VkRenderPass renderPass,
                                VkImageView imageView, uint32_t width,
                                uint32_t height);
bool createCommandPool(const VkDevice logicalDevice,
                       const VkCommandPoolCreateFlags parameters,
                       const uint32_t queueFamily, VkCommandPool& commandPool);

bool allocateCommandBuffers(const VkDevice logicalDevice,
                            const VkCommandPool commandPool,
                            const VkCommandBufferLevel level,
                            const uint32_t count,
                            std::vector<VkCommandBuffer>& commandBuffers);

bool presentImage(VkQueue queue, std::vector<VkSemaphore> renderingSemaphores,
                  std::vector<PresentInfo> imagesToPresent); 
VkPipeline
createGraphicsPipeline(VkDevice logicalDevice, VkShaderModule vs,
                       VkShaderModule ps, VkRenderPass renderPass,
                       VkPipelineVertexInputStateCreateInfo *vertexInfo, VkPipelineLayout& outLayout); 
 
} // namespace cp::graphics::vulkan
