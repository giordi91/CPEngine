
#include "CPEngine/platform/graphics/vulkan/vulkanRenderingContext.h"
#include "CPEngine/application.h"
#include "CPEngine/core/logging.h"
#include "CPEngine/globals.h"
#include "CPEngine/graphics/renderingContext.h"
#include "CPEngine/platform/graphics/vulkan/vulkanFunctions.h"
#include "cpVk.h"
#include <cassert>

namespace cp::graphics::vulkan {

bool endCommandBufferRecordingOperation(const VkCommandBuffer commandBuffer) {
  const VkResult result = vkEndCommandBuffer(commandBuffer);
  if (VK_SUCCESS != result) {
    logCoreError("Error occurred during command buffer recording.");
    return false;
  }
  return true;
}
bool submitCommandBuffersToQueue(
    VkQueue queue, std::vector<WaitSemaphoreInfo> waitSemaphoreInfos,
    std::vector<VkCommandBuffer> commandBuffers,
    std::vector<VkSemaphore> signalSemaphores, const VkFence fence) {
  std::vector<VkSemaphore> waitSemaphoreHandles;
  std::vector<VkPipelineStageFlags> waitSemaphoreStages;

  for (auto &waitSemaphoreInfo : waitSemaphoreInfos) {
    waitSemaphoreHandles.emplace_back(waitSemaphoreInfo.semaphore);
    waitSemaphoreStages.emplace_back(waitSemaphoreInfo.waitingStage);
  }

  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             nullptr,
                             static_cast<uint32_t>(waitSemaphoreInfos.size()),
                             waitSemaphoreHandles.data(),
                             waitSemaphoreStages.data(),
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data(),
                             static_cast<uint32_t>(signalSemaphores.size()),
                             signalSemaphores.data()};

  const VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
  if (VK_SUCCESS != result) {
    logCoreError("Error occurred during command buffer submission.");
    return false;
  }
  return true;
}
void setImageMemoryBarrier(
    const VkCommandBuffer commandBuffer,
    const VkPipelineStageFlags generatingStages,
    const VkPipelineStageFlags consumingStages,
    const std::vector<ImageTransition> imageTransitions) {
  std::vector<VkImageMemoryBarrier> imageMemoryBarriers;

  for (auto &imageTransition : imageTransitions) {
    imageMemoryBarriers.push_back(
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         nullptr,
         imageTransition.currentAccess,
         imageTransition.newAccess,
         imageTransition.currentLayout,
         imageTransition.newLayout,
         imageTransition.currentQueueFamily,
         imageTransition.newQueueFamily,
         imageTransition.image,
         {

             imageTransition.aspect, 0, VK_REMAINING_MIP_LEVELS, 0,
             VK_REMAINING_ARRAY_LAYERS}});
  }

  if (!imageMemoryBarriers.empty()) {
    vkCmdPipelineBarrier(commandBuffer, generatingStages, consumingStages, 0, 0,
                         nullptr, 0, nullptr,
                         static_cast<uint32_t>(imageMemoryBarriers.size()),
                         imageMemoryBarriers.data());
  }
}
bool acquireSwapchainImage(const VkDevice logicalDevice,
                           const VkSwapchainKHR swapchain,
                           const VkSemaphore semaphore, const VkFence fence,
                           uint32_t &imageIndex) {
  const VkResult result = vkAcquireNextImageKHR(
      logicalDevice, swapchain, 2000000000, semaphore, fence, &imageIndex);
  switch (result) {
  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    return true;
  default:
    return false;
  }
}
bool beginCommandBufferRecordingOperation(
    const VkCommandBuffer commandBuffer, const VkCommandBufferUsageFlags usage,
    VkCommandBufferInheritanceInfo *secondaryCommandBufferInfo) {
  VkCommandBufferBeginInfo commandBufferBeginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, usage,
      secondaryCommandBufferInfo};

  const VkResult result =
      vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
  if (VK_SUCCESS != result) {
    // std::cout << "Could not begin command buffer recording operation."
    //          << std::endl;
    logCoreError("Could not begin command buffer recording operation.");
    return false;
  }
  return true;
}

graphics::RenderingContext *
createVulkanRenderingContext(const RenderingContextCreationSettings &settings,
                             const uint32_t width, const uint32_t height) {
  return new VulkanRenderingContext(settings, width, height);
}

VulkanRenderingContext::VulkanRenderingContext(
    const RenderingContextCreationSettings &settings, const uint32_t width,
    const uint32_t height)
    : RenderingContext(settings, width, height) {
  logCoreInfo("Initializing a Vulkan context");
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
          instanceExtensions, "Vulkan Viewport", m_resources.instance)) {
    return false;
  }

  if (!loadInstanceLevelFunctions(m_resources.instance, instanceExtensions)) {
    return false;
  }

  registerDebugCallback(m_resources.instance);
  // registerDebugCallback2(m_resources.INSTANCE);

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
      m_resources.instance, &surfaceCreateInfo, nullptr, &m_resources.surface);

  assert(VK_SUCCESS == result);

  // Logical device creation
  std::vector<VkPhysicalDevice> physicalDevices;
  enumerateAvailablePhysicalDevices(m_resources.instance, physicalDevices);

  uint32_t graphicsQueueFamilyIndex = 0;
  uint32_t presentQueueFamilyIndex;
  for (auto &physicalDevice : physicalDevices) {
    if (!selectIndexOfQueueFamilyWithDesiredCapabilities(
            physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex)) {
      continue;
    }

    if (!selectQueueFamilyThatSupportsPresentationToGivenSurface(
            physicalDevice, m_resources.surface, presentQueueFamilyIndex)) {
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
            m_resources.logicalDevice)) {
      continue;
    } else {
      if (!loadDeviceLevelFunctions(m_resources.logicalDevice,
                                    deviceExtensions)) {
        continue;
      }
      m_resources.physicalDevice = physicalDevice;

      getDeviceQueue(m_resources.logicalDevice, graphicsQueueFamilyIndex, 0,
                     m_resources.graphicsQueue);
      getDeviceQueue(m_resources.logicalDevice, presentQueueFamilyIndex, 0,
                     m_resources.presentationQueue);
      break;
    }
  }

  assert(m_resources.logicalDevice != nullptr);
  // create swap
  // this is the swap chain to be filled, then the old one is null SWAP_CHAIN
  const auto swapchain = new VkSwapchain();
  createSwapchain(m_resources.logicalDevice, m_resources.physicalDevice,
                  m_resources.surface, m_settings.width, m_settings.height,
                  m_resources.swapChain, *swapchain, m_resources.renderPass,
                  m_resources.imageFormat);
  m_resources.swapChain = swapchain;

  if (!newSemaphore(m_resources.logicalDevice,
                    m_resources.imageAcquiredSemaphore)) {
    assert(0);
  }

  if (!newSemaphore(m_resources.logicalDevice,
                    m_resources.readyToPresentSemaphore)) {
    assert(0);
  }

  // Command buffers creation
  if (!createCommandPool(m_resources.logicalDevice,
                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                         graphicsQueueFamilyIndex, m_resources.commandPool)) {
    assert(0);
  }

  std::vector<VkCommandBuffer> commandBuffers;
  if (!allocateCommandBuffers(
          m_resources.logicalDevice, m_resources.commandPool,
          VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers)) {
    assert(0);
  }
  m_resources.commandBuffer = commandBuffers[0];

  // temporary initialization of render pass, descriptor stuff
  // tempPipeInit();

  return true;
}
bool VulkanRenderingContext::newFrame() {

  waitForAllSubmittedCommandsToBeFinished(m_resources.logicalDevice);

  if (!acquireSwapchainImage(m_resources.logicalDevice,
                             m_resources.swapChain->swapchain,
                             m_resources.imageAcquiredSemaphore, VK_NULL_HANDLE,
                             m_internalResourceIndex)) {
    return false;
  }
  if (!beginCommandBufferRecordingOperation(
          m_resources.commandBuffer,
          VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
    return false;
  }

  const ImageTransition imageTransitionBeforeDrawing = {
      m_resources.swapChain->images[m_internalResourceIndex],
      0,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_QUEUE_FAMILY_IGNORED, // used for cross queue sync
      VK_QUEUE_FAMILY_IGNORED,
      VK_IMAGE_ASPECT_COLOR_BIT}; // this wont work if you have depth buffers

  setImageMemoryBarrier(m_resources.commandBuffer,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        {imageTransitionBeforeDrawing});

  // temporary begin of render pass
  VkClearColorValue color{0.5f, 0.9f, 0.5f, 1.0f};
  VkClearValue clear{};
  clear.color = color;

  VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  beginInfo.renderPass = m_resources.renderPass;
  beginInfo.framebuffer =
      m_resources.swapChain->frameBuffers[m_internalResourceIndex];

  // similar to a viewport mostly used on "tiled renderers" to optimize, talking
  // about hardware based tile renderer, aka mobile GPUs.
  // TODO change this should not live in application possibly
  beginInfo.renderArea.extent.width = static_cast<int32_t>(m_screenInfo.width);
  beginInfo.renderArea.extent.height =
      static_cast<int32_t>(m_screenInfo.height);
  beginInfo.clearValueCount = 1;
  beginInfo.pClearValues = &clear;

  vkCmdBeginRenderPass(m_resources.commandBuffer, &beginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  return true;
}

bool VulkanRenderingContext::dispatchFrame() {
  vkCmdEndRenderPass(m_resources.commandBuffer);

  const ImageTransition imageTransitionBeforePresent = {
      m_resources.swapChain->images[m_internalResourceIndex],
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_ACCESS_MEMORY_READ_BIT,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      VK_IMAGE_ASPECT_COLOR_BIT};
  setImageMemoryBarrier(m_resources.commandBuffer,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        {imageTransitionBeforePresent});

  if (!endCommandBufferRecordingOperation(m_resources.commandBuffer)) {
    return false;
  }

  const WaitSemaphoreInfo waitSemaphoreInfo = {
      m_resources.imageAcquiredSemaphore, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
  if (!submitCommandBuffersToQueue(
          m_resources.presentationQueue, {waitSemaphoreInfo},
          {m_resources.commandBuffer}, {m_resources.readyToPresentSemaphore},
          VK_NULL_HANDLE)) {
    return false;
  }

  const PresentInfo presentInfo = {m_resources.swapChain->swapchain,
                                   m_internalResourceIndex};
  const bool res =
      presentImage(m_resources.presentationQueue,
                   {m_resources.readyToPresentSemaphore}, {presentInfo});
  return res;
}

bool VulkanRenderingContext::resize(const uint32_t width,
                                    const uint32_t height) {
  m_screenInfo = {width, height};
  resizeSwapchain(m_resources.logicalDevice, m_resources.physicalDevice,
                  m_resources.surface, width, height, *(m_resources.swapChain),
                  m_resources.renderPass, m_resources.imageFormat);
  return true;
}

void VulkanRenderingContext::tempPipeInit() {
  // if constexpr (!USE_PUSH) {
  // createDescriptorPool(m_resources.logicalDevice, {10000, 10000}, m_dPool);
  //}

  // load the shaders
  // m_vs = loadShader(LOGICAL_DEVICE,
  // "../data/compiled/triangle.vert.glsl.spv"); assert(m_vs); m_fs =
  // loadShader(LOGICAL_DEVICE, "../data/compiled/triangle.frag.glsl.spv");
  // assert(m_fs);

  // load mesh
  // loadMesh("../data/lucy.obj", m_mesh);

  // allocate memory buffer for the mesh
  // VkPhysicalDeviceMemoryProperties memoryRequirements;
  // vkGetPhysicalDeviceMemoryProperties(PHYSICAL_DEVICE, &memoryRequirements);
  // createBuffer(
  //    m_vertexBuffer, LOGICAL_DEVICE, memoryRequirements, 128 * 1024 * 1024,
  //    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  // createBuffer(
  //    m_indexBuffer, LOGICAL_DEVICE, memoryRequirements, 128 * 1024 * 1024,
  //    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  // assert(m_vertexBuffer.size >= m_mesh.vertices.size() * sizeof(Vertex));
  // assert(m_indexBuffer.size >= m_mesh.indices.size() * sizeof(uint32_t));

  // memcpy(m_vertexBuffer.data, m_mesh.vertices.data(),
  //       m_mesh.vertices.size() * sizeof(Vertex));
  // memcpy(m_indexBuffer.data, m_mesh.indices.data(),
  //       m_mesh.indices.size() * sizeof(uint32_t));

  // SET_DEBUG_NAME(m_vertexBuffer.buffer, VK_OBJECT_TYPE_BUFFER, "vertex
  // buffer"); SET_DEBUG_NAME(m_indexBuffer.buffer, VK_OBJECT_TYPE_BUFFER,
  // "index buffer"); m_resources.pipeline =
  //    createGraphicsPipeline(m_resources.logicalDevice, nullptr, nullptr,
  //    m_resources.renderPass, nullptr,m_resources.pipelineLayout);

  // loadTextureFromFile("../data/uv.DDS", VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
  //                    LOGICAL_DEVICE, uvTexture);

  // if constexpr (!USE_PUSH) {
  // createDescriptorLayoutAdvanced();
  // createDescriptorLayoutAdvanced();
  //}
}

} // namespace cp::graphics::vulkan
