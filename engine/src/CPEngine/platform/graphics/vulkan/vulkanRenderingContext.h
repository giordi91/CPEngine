#pragma once
#include "CPEngine/graphics/renderingContext.h"
#include <vulkan/vulkan.h>
// TODO: remove vector
#include "CPEngine/platform/graphics/vulkan/vkSwapChain.h"
#include <vector>

namespace cp::graphics::vulkan {

struct VulkanResources {
  VkInstance instance = nullptr;
  VkSurfaceKHR surface = nullptr;
  VkDevice logicalDevice = nullptr;
  VkQueue graphicsQueue = nullptr;
  VkQueue computeQueue = nullptr;
  VkQueue presentationQueue = nullptr;
  VkPhysicalDevice physicalDevice = nullptr;
  VkSwapchain *swapChain = nullptr;
  VkRenderPass renderPass = nullptr;
  VkSemaphore imageAcquiredSemaphore = nullptr;
  VkSemaphore readyToPresentSemaphore = nullptr;
  VkCommandPool commandPool = nullptr;
  VkCommandBuffer commandBuffer = nullptr;
  VkFormat imageFormat = VK_FORMAT_UNDEFINED;
  VkPipelineLayout pipelineLayout = nullptr;
  VkPipeline pipeline = nullptr;
  std::vector<VkDescriptorSetLayout> descriptorSetLayoutToDelete;
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
  void tempPipeInit();
private:
  VulkanResources m_resources{};
  uint32_t m_internalResourceIndex = 0;
};

} // namespace cp::graphics::vulkan