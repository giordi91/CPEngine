
#include "CPEngine/platform/graphics/vulkan/vulkanRenderingContext.h"
#include "CPEngine/application.h"
#include "CPEngine/core/logging.h"
#include "CPEngine/graphics/renderingContext.h"
#include "VulkanFunctions.h"
#include "cpVk.h"
#include <CPEngine/globals.h>
#include <cassert>

namespace cp::graphics::vulkan {

graphics::RenderingContext *
createVulkanRenderingContext(const RenderingContextCreationSettings &settings,
                             const uint32_t width, const uint32_t height) {
  return new VulkanRenderingContext(settings, width, height);
}

VulkanRenderingContext::VulkanRenderingContext(
    const RenderingContextCreationSettings &settings, const uint32_t width,
    const uint32_t height)
    : RenderingContext(settings, width, height) {
  logCoreInfo("Initializing a DirectX 12 context");
}

bool VulkanRenderingContext::initializeGraphics() {
  // lets enable debug layer if needed
#if defined(DEBUG) || defined(_DEBUG)
  {}
#endif

  VULKAN_LIBRARY = LoadLibrary(L"vulkan-1.dll");
  assert(VULKAN_LIBRARY != nullptr);

  if (!loadFunctionExportedFromVulkanLoaderLibrary(VULKAN_LIBRARY)) {
    return false;
  }

  if (!loadGlobalLevelFunctions()) {
    return false;
  }

  std::vector<char const *> instanceExtensions;
  if (!createVulkanInstanceWithWsiExtensionsEnabled(
          instanceExtensions, "Vulkan Viewport", m_resources.INSTANCE)) {
    return false;
  }

  if (!loadInstanceLevelFunctions(m_resources.INSTANCE, instanceExtensions)) {
    return false;
  }

  registerDebugCallback(m_resources.INSTANCE);

  const core::NativeWindow *windowData = m_settings.window->getNativeWindow();
#if CP_WINDOWS_PLATFORM
  HWND window = nullptr;
  HINSTANCE hinstance = nullptr;
  memcpy(&window, &windowData->data2, sizeof(HWND));
  memcpy(&hinstance, &windowData->data, sizeof(HINSTANCE));

  // init swap chain
  VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
      VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, hinstance,
      window};
#else
  assert(!"ONLY WINDOWS SUPPORTED");
#endif

  const VkResult result = vkCreateWin32SurfaceKHR(
      m_resources.INSTANCE, &surfaceCreateInfo, nullptr, &m_resources.SURFACE);

  assert(VK_SUCCESS == result);

  // Logical device creation
  std::vector<VkPhysicalDevice> physicalDevices;
  enumerateAvailablePhysicalDevices(m_resources.INSTANCE, physicalDevices);

  uint32_t graphicsQueueFamilyIndex = 0;
  uint32_t presentQueueFamilyIndex;
  for (auto &physicalDevice : physicalDevices) {
    if (!selectIndexOfQueueFamilyWithDesiredCapabilities(
            physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex)) {
      continue;
    }

    if (!selectQueueFamilyThatSupportsPresentationToGivenSurface(
            physicalDevice, m_resources.SURFACE, presentQueueFamilyIndex)) {
      continue;
    }

    std::vector<QueueInfo> requestedQueues = {
        {graphicsQueueFamilyIndex, {1.0f}}};
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
      requestedQueues.push_back({presentQueueFamilyIndex, {1.0f}});
    }
    std::vector<char const *> deviceExtensions;

    VkPhysicalDevice8BitStorageFeaturesKHR feature8{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR};
    feature8.storageBuffer8BitAccess = true;
    feature8.uniformAndStorageBuffer8BitAccess = true;

    VkPhysicalDeviceFeatures2 deviceFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    deviceFeatures.features = {};
    deviceFeatures.features.vertexPipelineStoresAndAtomics = true;
    deviceFeatures.features.geometryShader = true;

    deviceFeatures.pNext = &feature8;

    if (!createLogicalDeviceWithWsiExtensionsEnabled(
            physicalDevice, requestedQueues, deviceExtensions, &deviceFeatures,
            m_resources.LOGICAL_DEVICE)) {
      continue;
    } else {
      if (!loadDeviceLevelFunctions(m_resources.LOGICAL_DEVICE,
                                    deviceExtensions)) {
        continue;
      }
      m_resources.PHYSICAL_DEVICE = physicalDevice;

      /*
      getDeviceQueue(m_resources.LOGICAL_DEVICE, graphicsQueueFamilyIndex, 0,
                     m_resources.GRAPHICS_QUEUE);
      getDeviceQueue(m_resources.LOGICAL_DEVICE, presentQueueFamilyIndex, 0,
                     m_resources.PRESENTATION_QUEUE);
                     */
      break;
    }
  }

  /*
  assert(LOGICAL_DEVICE != nullptr);
  // create swap
  const auto swapchain = new Swapchain();
  createSwapchain(LOGICAL_DEVICE, PHYSICAL_DEVICE, SURFACE,
                  globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, SWAP_CHAIN,
                  *swapchain, RENDER_PASS);
  SWAP_CHAIN = swapchain;

  if (!newSemaphore(LOGICAL_DEVICE, IMAGE_ACQUIRED_SEMAPHORE)) {
    assert(0);
  }

  if (!newSemaphore(LOGICAL_DEVICE, READY_TO_PRESENT_SEMAPHORE)) {
    assert(0);
  }

  // Command buffers creation
  if (!createCommandPool(LOGICAL_DEVICE,
                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                         graphicsQueueFamilyIndex, COMMAND_POOL)) {
    assert(0);
  }

  std::vector<VkCommandBuffer> commandBuffers;
  if (!allocateCommandBuffers(LOGICAL_DEVICE, COMMAND_POOL,
                              VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1,
                              commandBuffers)) {
    assert(0);
  }
  COMMAND_BUFFER = commandBuffers[0];

  */
  return true;
} // namespace cp::graphics::dx12
bool VulkanRenderingContext::newFrame() { return true; }

bool VulkanRenderingContext::dispatchFrame() { return true; }

bool VulkanRenderingContext::resize(const uint32_t width,
                                    const uint32_t height) {
  return true;
}

} // namespace cp::graphics::vulkan
