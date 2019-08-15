#pragma once
//#include "CPEngine/platform/windows/graphics/dx12/cpDx12.h"
//#include "platform/windows/graphics/dx12/TextureManagerDx12.h"
#include "CPEngine/platform/graphics/dx12/cpDX12.h"
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>

namespace cp::graphics {
namespace dx12 {
struct FrameCommand;
class Dx12RenderingContext;

// frame-buffer configuration, hard-coded for the time being
// const DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
// const DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

extern DXGI_FORMAT BACKBUFFER_FORMAT;
extern DXGI_FORMAT DEPTHSTENCIL_FORMAT;

struct Dx12SwapChain {

  IDXGISwapChain *swapChain = nullptr;
  UINT m_currentBackBuffer = 0;
  uint32_t m_inFlightFrames = 0;
  // TextureHandle m_swapChainBuffersHandles[FRAME_BUFFERS_COUNT];
  // DescriptorPair m_swapChainBuffersDescriptors[FRAME_BUFFERS_COUNT];
  ID3D12Resource *m_swapChainBuffersHandles[2];
  DescriptorPair m_swapChainBuffersDescriptors[2];
  // TextureHandle m_swapChainDepth;
  // DescriptorPair m_swapChainDepthDescriptors;
  ID3D12Resource *m_swapChainDepth = nullptr;
  DescriptorPair m_swapChainDepthDescriptors{};
  D3D12_VIEWPORT m_screenViewport{};
  D3D12_RECT m_scissorRect{};
  bool m_isInit = false;
};

bool createSwapchain(Dx12SwapChain *swapchain,
                     const Dx12RenderingContext *context);
bool resizeSwapchain(Dx12RenderingContext *context, Dx12SwapChain *swapchain,
                     FrameCommand *command, const int width, const int height);

inline D3D12_CPU_DESCRIPTOR_HANDLE
swapchainCurrentBackBufferView(Dx12SwapChain *swapchain) {
  return swapchain
      ->m_swapChainBuffersDescriptors[swapchain->m_currentBackBuffer]
      .cpuHandle;
}

inline ID3D12Resource *swapchainCurrentBackBufferTexture(Dx12SwapChain *swapchain) {
  return swapchain->m_swapChainBuffersHandles[swapchain->m_currentBackBuffer];
}
inline D3D12_CPU_DESCRIPTOR_HANDLE
swapChainGetDepthCPUDescriptor(Dx12SwapChain *swapchain) {
  return swapchain->m_swapChainDepthDescriptors.cpuHandle;
}
inline void swapchainPresent(Dx12SwapChain *swapchain) {
  swapchain->swapChain->Present(0, 0);
  swapchain->m_currentBackBuffer =
      (swapchain->m_currentBackBuffer + 1) % swapchain->m_inFlightFrames;
}
} // namespace dx12
} // namespace cp::graphics
