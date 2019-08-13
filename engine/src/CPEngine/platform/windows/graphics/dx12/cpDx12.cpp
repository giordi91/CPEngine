
#include "d3dx12.h"
#if CP_WINDOWS_PLATFORM

#include "CPEngine/application.h"
#include "CPEngine/core/logging.h"
#include "CPEngine/graphics/renderingContext.h"
#include "CPEngine/platform/windows/graphics/dx12/cpDx12.h"
#include "descriptorHeap.h"
#include "tempDefinition.h"
#include <CPEngine/globals.h>
#include <cassert>
#include <d3d12.h>
#include <minwindef.h>

namespace cp::graphics::dx12 {

void createFrameCommand(D3D12DeviceType *device, FrameCommand *fc) {
  auto result = device->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fc->commandAllocator));
  assert(SUCCEEDED(result));

  result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     fc->commandAllocator, nullptr,
                                     IID_PPV_ARGS(&fc->commandList));
  assert(SUCCEEDED(result));
  fc->commandList->Close();
  fc->isListOpen = false;
}

inline HRESULT resetAllocatorAndList(FrameCommand *command) {

  assert(!command->isListOpen);

  // Reuse the memory associated with command recording.
  // We can only reset when the associated command lists have finished
  // execution on the GPU.
  const HRESULT result = command->commandAllocator->Reset();
  assert(SUCCEEDED(result) && "failed resetting allocator");

  // A command list can be reset after it has been added to the command queue
  // via ExecuteCommandList.
  // Reusing the command list reuses memory.
  const HRESULT result2 =
      command->commandList->Reset(command->commandAllocator, nullptr);
  assert(SUCCEEDED(result) && "failed resetting allocator");
  command->isListOpen = SUCCEEDED(result) & SUCCEEDED(result2);
  return result2;
}

void Dx12RenderingContext::flushGlobalCommandQueue() {
  flushCommandQueue(m_resources.globalCommandQueue);
}

void Dx12RenderingContext::flushCommandQueue(ID3D12CommandQueue *queue) {
  // Advance the fence value to mark commands up to this fence point.
  ++m_internalCurrentFence;

  // Add an instruction to the command queue to set a new fence point. Because
  // we are on the GPU time line, the new fence point won't be set until the
  // GPU finishes processing all the commands prior to this Signal().
  HRESULT res = queue->Signal(m_resources.globalFence, m_internalCurrentFence);
  assert(SUCCEEDED(res));
  auto id = m_resources.globalFence->GetCompletedValue();
  // Wait until the GPU has completed commands up to this fence point.
  if (id < m_internalCurrentFence) {
    const HANDLE eventHandle =
        CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

    // Fire event when GPU hits current fence.
    res = m_resources.globalFence->SetEventOnCompletion(m_internalCurrentFence,
                                                        eventHandle);
    assert(SUCCEEDED(res));

    // Wait until the GPU hits current fence event is fired.
    WaitForSingleObject(eventHandle, INFINITE);
    CloseHandle(eventHandle);
  }
}

graphics::RenderingContext *
createDx12RenderingContext(const RenderingContextCreationSettings &settings) {
  return new Dx12RenderingContext(settings);
}

Dx12RenderingContext::Dx12RenderingContext(
    const RenderingContextCreationSettings &settings)
    : RenderingContext(settings) {
  logCoreInfo("Initializing a DirectX 12 context");
}

bool Dx12RenderingContext::initializeGraphics() {
  // lets enable debug layer if needed
#if defined(DEBUG) || defined(_DEBUG)
  {
    const HRESULT result =
        D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController));
    if (FAILED(result)) {
      return false;
    }
    m_debugController->EnableDebugLayer();
    // ID3D12Debug1 *debug1;
    // DEBUG_CONTROLLER->QueryInterface(IID_PPV_ARGS(&debug1));
    // debug1->SetEnableGPUBasedValidation(true);
  }
#endif

  HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));
  if (FAILED(result)) {
    return false;
  }

  m_adapter = new Dx12Adapter();
#if DXR_ENABLED
  m_adapter->setFeture(AdapterFeature::DXR);
  m_adapter->setVendor(AdapterVendor::NVIDIA);
#else
  m_adapter->setFeature(ADAPTER_FEATURE::ANY);
  m_adapter->setVendor(ADAPTER_VENDOR::ANY);
#endif
  const bool found =
      m_adapter->findBestDx12Adapter(m_dxgiFactory);
  assert(found && "could not find adapter matching features");

  // log the adapter used
  auto *adapter = m_adapter->getDx12Adapter();
  DXGI_ADAPTER_DESC desc;
  const HRESULT adapterDescRes = SUCCEEDED(adapter->GetDesc(&desc));
  assert(SUCCEEDED(adapterDescRes));

  const char *toPrint = core::STRING_POOL->concatenateFrame(
      "Initializing graphics with adapter: ", desc.Description, "");
  logCoreInfo(toPrint);

  result = D3D12CreateDevice(m_adapter->getDx12Adapter(),
                             D3D_FEATURE_LEVEL_12_1,
                             IID_PPV_ARGS(&m_resources.device));
  if (FAILED(result)) {
    logCoreError("Could not create device with requested features");
    // falling back to WARP device
    IDXGIAdapter *warpAdapter;
    result =
        m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
    if (FAILED(result)) {
      return false;
    }

    result = D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_12_1,
                               IID_PPV_ARGS(&m_resources.device));
    if (FAILED(result)) {
      logCoreError(
          "Could not get device with requested features, not even warp");
      return false;
    }
  }

  // Check the maximum feature level, and make sure it's above our minimum
  D3D_FEATURE_LEVEL featureLevelsArray[1];
  featureLevelsArray[0] = D3D_FEATURE_LEVEL_12_1;
  D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels = {};
  featureLevels.NumFeatureLevels = 1;
  featureLevels.pFeatureLevelsRequested = featureLevelsArray;
  HRESULT r = m_resources.device->CheckFeatureSupport(
      D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels));
  assert(featureLevels.MaxSupportedFeatureLevel == D3D_FEATURE_LEVEL_12_1);
  assert(SUCCEEDED(r));

#if DXR_ENABLED
  if (m_resources.adapter->getFeature() == AdapterFeature::DXR) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts5 = {};
    dx12::DEVICE->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts5,
                                      sizeof(opts5));
    if (opts5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
      assert(0);
  }
#endif

  // creating the command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  const HRESULT qresult = m_resources.device->CreateCommandQueue(
      &queueDesc, IID_PPV_ARGS(&m_resources.globalCommandQueue));
  if (FAILED(qresult)) {
    return false;
  }

  result = m_resources.device->CreateFence(
      0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_resources.globalFence));
  if (FAILED(result)) {
    logCoreError("Could not create DX12 fence");
    return false;
  }

  if (m_resources.frameResources == nullptr) {
    m_resources.frameResources = new FrameResource[m_settings.inFlightFrames];
  }
  for (uint32_t i = 0; i < m_settings.inFlightFrames; ++i) {
    createFrameCommand(m_resources.device, &m_resources.frameResources[i].fc);
  }
  m_resources.currentFrameResource = &m_resources.frameResources[0];
  // creating global heaps
  m_resources.cbvSrvUavHeap = new DescriptorHeap();
  m_resources.cbvSrvUavHeap->initializeAsCBVSRVUAV(m_resources.device, 1000);

  m_resources.rtvHeap = new DescriptorHeap();
  m_resources.rtvHeap->initializeAsRTV(m_resources.device, 20);

  m_resources.dsvHeap = new DescriptorHeap();
  m_resources.dsvHeap->initializeAsDSV(m_resources.device, 20);

  /*

  // initialize the managers
  // TODO add initialize to all managers for consistency and symmetry
  IDENTITY_MANAGER = new IdentityManager();
  IDENTITY_MANAGER->initialize();
  CONSTANT_BUFFER_MANAGER = new ConstantBufferManagerDx12();
  CONSTANT_BUFFER_MANAGER->initialize();

  BUFFER_MANAGER = new BufferManagerDx12();
  BUFFER_MANAGER->initialize();

  globals::CONSTANT_BUFFER_MANAGER = CONSTANT_BUFFER_MANAGER;
  globals::BUFFER_MANAGER = BUFFER_MANAGER;
  TEXTURE_MANAGER = new TextureManagerDx12();
  TEXTURE_MANAGER->initialize();
  globals::TEXTURE_MANAGER = TEXTURE_MANAGER;
  MESH_MANAGER = new MeshManager();
  globals::ASSET_MANAGER = new AssetManager();
  globals::ASSET_MANAGER->initialize();
  globals::RENDERING_CONTEXT = new RenderingContext();
  globals::RENDERING_CONTEXT->initialize();

  SHADER_MANAGER = new ShaderManager();
  SHADER_MANAGER->init();
  SHADER_MANAGER->loadShadersInFolder(frameConcatenation(
      globals::DATA_SOURCE_PATH, "/processed/shaders/rasterization"));
  SHADER_MANAGER->loadShadersInFolder(frameConcatenation(
      globals::DATA_SOURCE_PATH, "/processed/shaders/compute"));

  ROOT_SIGNATURE_MANAGER = new RootSignatureManager();
  ROOT_SIGNATURE_MANAGER->loadSignaturesInFolder(
      frameConcatenation(globals::DATA_SOURCE_PATH, "/processed/rs"));

  SHADER_LAYOUT_REGISTRY = new dx12::ShadersLayoutRegistry();

  PSO_MANAGER = new PSOManager();
  PSO_MANAGER->init(dx12::DEVICE, SHADER_LAYOUT_REGISTRY,
                    ROOT_SIGNATURE_MANAGER, dx12::SHADER_MANAGER);
  PSO_MANAGER->loadPSOInFolder(
      frameConcatenation(globals::DATA_SOURCE_PATH, "/pso"));

  // mesh manager needs to load after pso and RS since it initialize material
  // types
  MATERIAL_MANAGER = new MaterialManager();

  MATERIAL_MANAGER->init();
  MATERIAL_MANAGER->loadTypesInFolder(
      frameConcatenation(globals::DATA_SOURCE_PATH, "/materials/types"));

  globals::DEBUG_FRAME_DATA = new globals::DebugFrameData();

  const bool isHeadless = (wnd == nullptr) | (width == 0) | (height == 0);

  if (!isHeadless) {

  return true;
*/
  // init swap chain
  m_resources.swapChain = new dx12::SwapChain();
  m_resources.swapChain->initialize(this);
  flushCommandQueue(m_resources.globalCommandQueue);
  m_resources.swapChain->resize(&m_resources.currentFrameResource->fc,
                                m_settings.width, m_settings.height);
  //}
  // else {
  //  SE_CORE_INFO("Requested HEADLESS client, no swapchain is initialized");
  //}

  return true;
} // namespace cp::graphics::dx12
bool Dx12RenderingContext::newFrame() {
  // here we need to check which frame resource we are going to use
  m_resources.currentFrameResource =
      &m_resources.frameResources[m_internalResourceIndex];

  // check if the resource has finished rendering if not we have to wait
  if (m_resources.currentFrameResource->fence != 0 &&
      m_resources.globalFence->GetCompletedValue() <
          m_resources.currentFrameResource->fence) {
    const HANDLE eventHandle =
        CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
    const auto handleResult = m_resources.globalFence->SetEventOnCompletion(
        m_resources.currentFrameResource->fence, eventHandle);
    assert(SUCCEEDED(handleResult));
    WaitForSingleObject(eventHandle, INFINITE);

    CloseHandle(eventHandle);
  }

  //resetting the command list and the allocator so is 
  //ready to be filled up again
  resetAllocatorAndList(&m_resources.currentFrameResource->fc);

  auto *commandList = m_resources.currentFrameResource->fc.commandList;
  // Reuse the memory associated with command recording.
  // We can only reset when the associated command lists have finished
  // execution on the GPU.
  // Indicate a state transition on the resource usage.
  D3D12_RESOURCE_BARRIER rtbarrier[1];
  ID3D12Resource *backBuffer =
      m_resources.swapChain->currentBackBufferTexture();
  rtbarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
      backBuffer, D3D12_RESOURCE_STATE_PRESENT,
      D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList->ResourceBarrier(1, rtbarrier);

  // Set the viewport and scissor rect.  This needs to be reset whenever the
  // command list is reset.
  commandList->RSSetViewports(1, m_resources.swapChain->getViewport());
  commandList->RSSetScissorRects(1, m_resources.swapChain->getScissorRect());

  //temporary color clear
  float gray[4] = {0.5f, 0.9f, 0.5f, 1.0f};
  auto backBufferDescriptor =m_resources.swapChain->currentBackBufferView();
  commandList->ClearRenderTargetView(backBufferDescriptor,gray,0,nullptr);

  auto *heap = m_resources.cbvSrvUavHeap->getResource();
  commandList->SetDescriptorHeaps(1, &heap);

  return true;
}

bool Dx12RenderingContext::dispatchFrame() {
  D3D12_RESOURCE_BARRIER rtbarrier[1];
  // finally transition the resource to be present
  auto *commandList = m_resources.currentFrameResource->fc.commandList;

  ID3D12Resource* backBuffer = m_resources.swapChain->currentBackBufferTexture();
  rtbarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
      backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_PRESENT);
  commandList->ResourceBarrier(1, rtbarrier);
  // TextureHandle backBufferH = dx12::SWAP_CHAIN->currentBackBufferTexture();
  // int rtcounter = dx12::TEXTURE_MANAGER->transitionTexture2DifNeeded(
  //    backBufferH, D3D12_RESOURCE_STATE_PRESENT, rtbarrier, 0);
  // if (rtcounter != 0) {
  //  commandList->ResourceBarrier(rtcounter, rtbarrier);
  //}

  // Done recording commands.
  dx12::executeCommandList(m_resources.globalCommandQueue,
                           &m_resources.currentFrameResource->fc);

  m_resources.currentFrameResource->fence = ++m_internalCurrentFence;
  m_resources.globalCommandQueue->Signal(m_resources.globalFence,
                                         m_internalCurrentFence);
  // swap the back and front buffers
  m_resources.swapChain->present();
  // bump the frame
  ++globals::CURRENT_FRAME;
  m_internalResourceIndex =
      (m_internalResourceIndex + 1) % m_settings.inFlightFrames;
  return true;
}
} // namespace cp::graphics::dx12
#endif
