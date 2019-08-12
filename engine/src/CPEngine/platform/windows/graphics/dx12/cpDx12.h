#pragma once
#include "CPEngine/graphics/renderingContext.h"
#include "CPEngine/platform/windows/graphics/dx12/Dx12Adapter.h"
#include "CPEngine/platform/windows/graphics/dx12/swapChain.h"
#include <d3d12.h>
#include <dxgi1_6.h>

namespace cp::graphics {
namespace dx12 {

struct FrameCommand final {
  ID3D12CommandAllocator *commandAllocator = nullptr;
#if DXR_ENABLED
  ID3D12GraphicsCommandList4 *commandList = nullptr;
#else
  ID3D12GraphicsCommandList2 *commandList = nullptr;
#endif
  bool isListOpen = false;
};

struct FrameResource final {
  FrameCommand fc;
  UINT64 fence = 0;
};

#if DXR_ENABLED
typedef ID3D12Device5 D3D12DeviceType;
#else
typedef ID3D12Device3 D3D12DeviceType;
#endif

struct Dx12Resources {
  D3D12DeviceType *device = nullptr;
  ID3D12Debug *debugController = nullptr;
  IDXGIFactory6 *dxgiFacotry = nullptr;
  Dx12Adapter *adapter = nullptr;
  ID3D12CommandQueue *globalCommandQueue = nullptr;
  ID3D12Fence *globalFence = nullptr;
  SwapChain *swapChain = nullptr;
  FrameResource *frameResources = nullptr;
  FrameResource *currentFrameResource = nullptr;
};

graphics::RenderingContext *
createDx12RenderingContext(const RenderingContextCreationSettings &settings);

class Dx12RenderingContext final : public RenderingContext {
public:
  explicit Dx12RenderingContext(
      const RenderingContextCreationSettings &settings);
  ~Dx12RenderingContext() = default;
  // private copy and assignment
  Dx12RenderingContext(const Dx12RenderingContext &) = delete;
  Dx12RenderingContext &operator=(const Dx12RenderingContext &) = delete;

  bool initializeGraphics() override;
  inline Dx12Resources *getResources() { return &m_resources; }

private:
  Dx12Resources m_resources;
  /*
  TextureManagerDx12 *TEXTURE_MANAGER = nullptr;
  MeshManager *MESH_MANAGER = nullptr;
  IdentityManager *IDENTITY_MANAGER = nullptr;
  MaterialManager *MATERIAL_MANAGER = nullptr;
  Graph *RENDERING_GRAPH = nullptr;
  ConstantBufferManagerDx12 *CONSTANT_BUFFER_MANAGER = nullptr;
  ShaderManager *SHADER_MANAGER = nullptr;
  PSOManager *PSO_MANAGER = nullptr;
  RootSignatureManager *ROOT_SIGNATURE_MANAGER = nullptr;
  ShadersLayoutRegistry *SHADER_LAYOUT_REGISTRY = nullptr;
  BufferManagerDx12 *BUFFER_MANAGER = nullptr;
  */
};

} // namespace dx12
} // namespace cp::graphics