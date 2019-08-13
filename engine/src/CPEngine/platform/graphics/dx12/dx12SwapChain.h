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

class SwapChain final {
public:
  SwapChain() = default;
  ~SwapChain() = default;
  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  bool resize(FrameCommand *command, int width, int height);
  bool initialize(Dx12RenderingContext *context);
  inline IDXGISwapChain *getSwapChain() const { return m_swapChain; }

  inline D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView() const {
    return m_swapChainBuffersDescriptors[m_currentBackBuffer].cpuHandle;
  }

  inline ID3D12Resource *currentBackBufferTexture() const {
    return m_swapChainBuffersHandles[m_currentBackBuffer];
  }

  inline D3D12_VIEWPORT *getViewport() { return &m_screenViewport; }
  inline D3D12_RECT *getScissorRect() { return &m_scissorRect; }
  inline void present() {
    m_swapChain->Present(0, 0);
    m_currentBackBuffer = (m_currentBackBuffer + 1) % m_inFlightFrames;
  }
  // void clearDepth() const {
  //  CURRENT_FRAME_RESOURCE->fc.commandList->ClearDepthStencilView(
  //      m_swapChainDepthDescriptors.cpuHandle,
  //      D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0,
  //      nullptr);
  //}

  inline D3D12_CPU_DESCRIPTOR_HANDLE getDepthCPUDescriptor() {
    return m_swapChainDepthDescriptors.cpuHandle;
  }

private:
  // frame-buffer configuration, hard-coded for the time being
  const DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
  const DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
  bool m_4xMsaaState = false;
  UINT m_msaaQuality;

  IDXGISwapChain *m_swapChain = nullptr;

  UINT m_currentBackBuffer = 0;
  uint32_t m_inFlightFrames;
  // TextureHandle m_swapChainBuffersHandles[FRAME_BUFFERS_COUNT];
  // DescriptorPair m_swapChainBuffersDescriptors[FRAME_BUFFERS_COUNT];

  ID3D12Resource *m_swapChainBuffersHandles[2];
  DescriptorPair m_swapChainBuffersDescriptors[2];

  // TextureHandle m_swapChainDepth;
  // DescriptorPair m_swapChainDepthDescriptors;

  ID3D12Resource *m_swapChainDepth;
  DescriptorPair m_swapChainDepthDescriptors;

  D3D12_VIEWPORT m_screenViewport;
  D3D12_RECT m_scissorRect;
  bool m_isInit = false;

  Dx12RenderingContext *m_renderingContext = nullptr;
};
} // namespace dx12
} // namespace cp::graphics
