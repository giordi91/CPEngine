#pragma once
#include "CPEngine/graphics/renderingContext.h"
#include <vulkan/vulkan.h>
//TODO: remove vector
#include <vector>
#include "CPEngine/platform/graphics/vulkan/vkSwapChain.h"

namespace cp::graphics::vulkan {

struct VulkanResources {
  VkInstance INSTANCE = nullptr;
  VkSurfaceKHR SURFACE = nullptr;
  VkDevice LOGICAL_DEVICE = nullptr;
  VkQueue GRAPHICS_QUEUE = nullptr;
  VkQueue COMPUTE_QUEUE = nullptr;
  VkQueue PRESENTATION_QUEUE = nullptr;
  VkPhysicalDevice PHYSICAL_DEVICE = nullptr;
  VkSwapchain *SWAP_CHAIN = nullptr;
  VkRenderPass RENDER_PASS = nullptr;
  VkSemaphore IMAGE_ACQUIRED_SEMAPHORE = nullptr;
  VkSemaphore READY_TO_PRESENT_SEMAPHORE = nullptr;
  VkCommandPool COMMAND_POOL = nullptr;
  VkCommandBuffer COMMAND_BUFFER = nullptr;
  VkFormat IMAGE_FORMAT = VK_FORMAT_UNDEFINED;
  VkPipelineLayout PIPELINE_LAYOUT = nullptr;
  VkDebugReportCallbackEXT DEBUG_CALLBACK = nullptr;
  VkDebugUtilsMessengerEXT DEBUG_CALLBACK2 = nullptr;
  std::vector<VkDescriptorSetLayout> LAYOUTS_TO_DELETE;
};

graphics::RenderingContext *
createVulkanRenderingContext(const RenderingContextCreationSettings &settings,
                             uint32_t width, uint32_t height);

class VulkanRenderingContext final : public RenderingContext {
public:
  explicit VulkanRenderingContext(
      const RenderingContextCreationSettings &settings, uint32_t width,
      uint32_t height);
  ~VulkanRenderingContext() = default;
  // private copy and assignment
  VulkanRenderingContext(const VulkanRenderingContext &) = delete;
  VulkanRenderingContext &operator=(const VulkanRenderingContext &) = delete;

  bool initializeGraphics() override;
  bool newFrame() override;
  bool dispatchFrame() override;
  bool resize(uint32_t width, uint32_t height) override;

  inline VulkanResources *getResources() { return &m_resources; }

private:
private:
  VulkanResources m_resources{};
};

} // namespace cp::graphics::vulkan