#include "CPEngine/platform/windows/graphics/dx12/cpDx12.h"
#include "CPEngine/application.h"
#include "CPEngine/core/logging.h"
#include "CPEngine/graphics/renderingContext.h"
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
        D3D12GetDebugInterface(IID_PPV_ARGS(&m_resources.debugController));
    if (FAILED(result)) {
      return false;
    }
    m_resources.debugController->EnableDebugLayer();
    // ID3D12Debug1 *debug1;
    // DEBUG_CONTROLLER->QueryInterface(IID_PPV_ARGS(&debug1));
    // debug1->SetEnableGPUBasedValidation(true);
  }
#endif

  HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&m_resources.dxgiFacotry));
  if (FAILED(result)) {
    return false;
  }

  m_resources.adapter = new Dx12Adapter();
#if DXR_ENABLED
  m_adapter->setFeture(AdapterFeature::DXR);
  m_adapter->setVendor(AdapterVendor::NVIDIA);
#else
  m_resources.adapter->setFeature(ADAPTER_FEATURE::ANY);
  m_resources.adapter->setVendor(ADAPTER_VENDOR::ANY);
#endif
  const bool found =
      m_resources.adapter->findBestDx12Adapter(m_resources.dxgiFacotry);
  assert(found && "could not find adapter matching features");

  // log the adapter used
  auto *adapter = m_resources.adapter->getDx12Adapter();
  DXGI_ADAPTER_DESC desc;
  const HRESULT adapterDescRes = SUCCEEDED(adapter->GetDesc(&desc));
  assert(SUCCEEDED(adapterDescRes));

  const char *toPrint = core::STRING_POOL->concatenateFrame(
      "Initializing graphics with adapter: ", desc.Description, "");
  logCoreInfo(toPrint);

  result = D3D12CreateDevice(m_resources.adapter->getDx12Adapter(),
                             D3D_FEATURE_LEVEL_12_1,
                             IID_PPV_ARGS(&m_resources.device));
  if (FAILED(result)) {
    logCoreError("Could not create device with requested features");
    // falling back to WARP device
    IDXGIAdapter *warpAdapter;
    result =
        m_resources.dxgiFacotry->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
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
  /*
  // creating global heaps
  GLOBAL_CBV_SRV_UAV_HEAP = new DescriptorHeap();
  GLOBAL_CBV_SRV_UAV_HEAP->initializeAsCBVSRVUAV(1000);

  GLOBAL_RTV_HEAP = new DescriptorHeap();
  GLOBAL_RTV_HEAP->initializeAsRTV(20);

  GLOBAL_DSV_HEAP = new DescriptorHeap();
  GLOBAL_DSV_HEAP->initializeAsDSV(20);


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
  // dx12::flushCommandQueue(dx12::GLOBAL_COMMAND_QUEUE);
  // dx12::SWAP_CHAIN->resize(&dx12::CURRENT_FRAME_RESOURCE->fc, width, height);
  //}
  // else {
  //  SE_CORE_INFO("Requested HEADLESS client, no swapchain is initialized");
  //}

  return true;
} // namespace cp::graphics::dx12
} // namespace cp::graphics::dx12
