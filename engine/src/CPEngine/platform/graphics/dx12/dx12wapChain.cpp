#if CP_WINDOWS_PLATFORM

#include "CPEngine/platform/windows/core/windowsWindow.h"
#include "CPEngine/platform/graphics/dx12/dx12SwapChain.h"
#include "CPEngine/platform/graphics/dx12/cpDx12.h"
#include "CPEngine/platform/graphics/dx12/d3dx12.h"
#include "CPEngine/platform/graphics/dx12/descriptorHeap.h"
#include "CPEngine/platform/graphics/dx12/dx12RenderingContext.h"
#include <cassert>

//#include "platform/windows/graphics/dx12/DX12.h"
/*
#include "platform/windows/graphics/dx12/d3dx12.h"
#include "platform/windows/graphics/dx12/descriptorHeap.h"
#include <d3d12.h>
*/

namespace cp::graphics::dx12 {

namespace SwapChainConstants {
const char *BACK_BUFFER_NAMES[3]{"backBuffer1", "backBuffer2", "backBuffer3"};

}

//TODO temporary until we have a texture manager back in the business
DescriptorPair initializeFromResourceDx12(D3D12DeviceType *device,
                                          ID3D12Resource *resource,
                                          const char *name,
                                          const D3D12_RESOURCE_STATES state,
                                          DescriptorHeap *heap) {
  // since we are passing one resource, by definition the resource is static
  // data is now loaded need to create handle etc
  DescriptorPair pair;
  createRTVSRV(device, heap, resource, pair);

  // m_nameToHandle[name] = handle;
  // return handle;
  return pair;
}

//TODO temporary until we have a texture manager back in the business
ID3D12Resource *createDepthTexture(D3D12DeviceType *device, const char *name,
                                   const uint32_t width, const uint32_t height,
                                   const D3D12_RESOURCE_STATES state) {
  const bool m_4xMsaaState = false;

  // Create the depth/stencil buffer and view.
  D3D12_RESOURCE_DESC depthStencilDesc;
  depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  depthStencilDesc.Alignment = 0;
  depthStencilDesc.Width = width;
  depthStencilDesc.Height = height;
  depthStencilDesc.DepthOrArraySize = 1;
  depthStencilDesc.MipLevels = 1;

  // Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to
  // read from the depth buffer.  Therefore, because we need to create two views
  // to the same resource:
  //   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
  //   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
  // we need to create the depth buffer resource with a typeless format.
  depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

  // Check 4X MSAA quality support for our back buffer format.
  // All Direct3D 11 capable devices support 4X MSAA for all render
  // target formats, so we only need to check quality support.
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
  msQualityLevels.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
  msQualityLevels.SampleCount = 4;
  msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  msQualityLevels.NumQualityLevels = 0;
  HRESULT res =
      device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                  &msQualityLevels, sizeof(msQualityLevels));
  assert(SUCCEEDED(res));
  const UINT m_msaaQuality = msQualityLevels.NumQualityLevels;

  depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
  depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_msaaQuality - 1) : 0;
  depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  ID3D12Resource *resource;
  D3D12_CLEAR_VALUE optClear;
  optClear.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
  optClear.DepthStencil.Depth = 0.0f;
  optClear.DepthStencil.Stencil = 0;
  auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  res = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                                        &depthStencilDesc, state, &optClear,
                                        IID_PPV_ARGS(&resource));
  assert(SUCCEEDED(res));

  return resource;
}
/*
SwapChain::~SwapChain() {
  int FRAME_BUFFERS_COUNT = 2;
  for (int i = 0; i < FRAME_BUFFERS_COUNT; ++i) {
    // dx12::TEXTURE_MANAGER->free(m_swapChainBuffersHandles[i]);
  }
  // dx12::TEXTURE_MANAGER->free(m_swapChainDepth);
}
*/
bool SwapChain::initialize(Dx12RenderingContext *context) {

  m_renderingContext = context;
  Dx12Resources *resources = context->getResources();

  const auto &settings = context->getContextSettings();
  m_inFlightFrames = settings.inFlightFrames;

  const core::NativeWindow *windowData = settings.window->getNativeWindow();
  HWND window;
  memcpy(&window, &windowData->data2, sizeof(HWND));

  // Check 4X MSAA quality support for our back buffer format.
  // All Direct3D 11 capable devices support 4X MSAA for all render
  // target formats, so we only need to check quality support.
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
  msQualityLevels.Format = m_backBufferFormat;
  msQualityLevels.SampleCount = 4;
  msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  msQualityLevels.NumQualityLevels = 0;
  HRESULT result = resources->device->CheckFeatureSupport(
      D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels,
      sizeof(msQualityLevels));
  assert(SUCCEEDED(result));

  m_msaaQuality = msQualityLevels.NumQualityLevels;
  assert(m_msaaQuality > 0 && "Unexpected MSAA quality level.");

  // Release previous swap chain
  DXGI_SWAP_CHAIN_DESC swapDesc;
  swapDesc.BufferDesc.Width = settings.width;
  swapDesc.BufferDesc.Height = settings.height;
  swapDesc.BufferDesc.RefreshRate.Numerator = 0;
  swapDesc.BufferDesc.RefreshRate.Denominator = 1;
  swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  // if we got a valid MSAA state we set 4 samples
  swapDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
  swapDesc.SampleDesc.Quality = m_4xMsaaState ? m_msaaQuality - 1 : 0;
  swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  // double buffering
  swapDesc.BufferCount = settings.inFlightFrames;
  swapDesc.OutputWindow = window;
  swapDesc.Windowed = 1;
  swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // the reason why we pass a queue is because when the swap chain flushes uses
  // the queue
  result = m_renderingContext->getFactory()->CreateSwapChain(
      resources->globalCommandQueue, &swapDesc, &m_swapChain);

  return FAILED(result);
}

bool SwapChain::resize(FrameCommand *command, const int width,
                       const int height) {

  // Flush before changing any resources.
  m_renderingContext->flushGlobalCommandQueue();
  auto *resources = m_renderingContext->getResources();
  HRESULT result = resetCommandList(command);
  assert(SUCCEEDED(result));

  const RenderingContextCreationSettings &settings =
      m_renderingContext->getContextSettings();
  if (m_isInit) {
    for (uint32_t i = 0; i < settings.inFlightFrames; ++i) {
      // dx12::TEXTURE_MANAGER->free(m_swapChainBuffersHandles[i]);
      m_swapChainBuffersHandles[i]->Release();
    }
  }

  // Resize the swap chain.
  result = m_swapChain->ResizeBuffers(settings.inFlightFrames, width, height,
                                      m_backBufferFormat,
                                      DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
  assert(SUCCEEDED(result) && "failed to resize swap chain");

  // resetting the current back buffer
  m_currentBackBuffer = 0;

  for (UINT i = 0; i < settings.inFlightFrames; i++) {
    ID3D12Resource *resource;
    const HRESULT res = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(res));

    assert(i < 3 && "not enough back buffer names");
    // m_swapChainBuffersHandles[i] = initializeFromResourceDx12(
    //    resource, SwapChainConstants::BACK_BUFFER_NAMES[i],
    //    D3D12_RESOURCE_STATE_PRESENT);
    m_swapChainBuffersHandles[i] = resource;

    m_swapChainBuffersDescriptors[i] = initializeFromResourceDx12(
        resources->device, resource, "", D3D12_RESOURCE_STATE_COMMON,
        resources->rtvHeap);
  }

  // freeing depth and re-creating it;
  if (m_isInit) {
    // dx12::TEXTURE_MANAGER->free(m_swapChainDepth);
  }

  m_swapChainDepth = createDepthTexture(resources->device, "depthBuffer", width,
                                        height, D3D12_RESOURCE_STATE_COMMON);
  createDSV(resources->device, resources->dsvHeap, m_swapChainDepth,
            m_swapChainDepthDescriptors, DXGI_FORMAT_D32_FLOAT_S8X24_UINT);

  D3D12_RESOURCE_BARRIER barrier[1];

  // TEXTURE_MANAGER->transitionTexture2DifNeeded(
  //    m_swapChainDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, barrier, 0);
  barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
      m_swapChainDepth, D3D12_RESOURCE_STATE_COMMON,
      D3D12_RESOURCE_STATE_DEPTH_WRITE);
  command->commandList->ResourceBarrier(1, barrier);

  // Execute the resize commands.
  executeCommandList(resources->globalCommandQueue, command);

  // Wait until resize is complete.
  m_renderingContext->flushGlobalCommandQueue();

  // Update the viewport transform to cover the client area.
  m_screenViewport.TopLeftX = 0;
  m_screenViewport.TopLeftY = 0;
  m_screenViewport.Width = static_cast<float>(width);
  m_screenViewport.Height = static_cast<float>(height);
  m_screenViewport.MinDepth = 0.0f;
  m_screenViewport.MaxDepth = 1.0f;

  m_scissorRect = {0, 0, width, height};
  m_isInit = true;

  return true;
}

} // namespace cp::graphics::dx12
#endif
