#include <vector>
#include <vulkan/vulkan.h>

namespace cp::graphics::vulkan {

struct VkSwapchain {
  VkSwapchainKHR swapchain = nullptr;
  std::vector<VkImage> images;
  std::vector<VkImageView> imagesView;
  std::vector<VkFramebuffer> frameBuffers;
  uint32_t width = 0;
  uint32_t height = 0;
};

bool createSwapchain(const VkDevice logicalDevice,
                     const VkPhysicalDevice physicalDevice,
                     VkSurfaceKHR surface, uint32_t width, uint32_t height,
                     VkSwapchain *oldSwapchain, VkSwapchain &outSwapchain,
                     VkRenderPass &renderPass,VkFormat&  format);

bool destroySwapchain(const VkDevice logicalDevice, VkSwapchain *swapchain);

void resizeSwapchain(const VkDevice logicalDevice,
                     const VkPhysicalDevice physicalDevice,
                     VkSurfaceKHR surface, uint32_t width, uint32_t height,
                     VkSwapchain &outSwapchain, VkRenderPass &renderPass,
                     VkFormat &imageFormat);
} // namespace cp::graphics::vulkan