#pragma once
#include "CPEngine/graphics/renderingContext.h"
#include "CPEngine/platform/windows/graphics/dx12/Dx12Adapter.h"
#include "CPEngine/platform/windows/graphics/dx12/swapChain.h"
#include "tempDefinition.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>

namespace cp::graphics::dx12 {
class DescriptorHeap;

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

struct Dx12Resources {
  D3D12DeviceType *device = nullptr;
  ID3D12CommandQueue *globalCommandQueue = nullptr;
  ID3D12Fence *globalFence = nullptr;
  SwapChain *swapChain = nullptr;
  FrameResource *frameResources = nullptr;
  FrameResource *currentFrameResource = nullptr;
  DescriptorHeap *cbvSrvUavHeap = nullptr;
  DescriptorHeap *rtvHeap = nullptr;
  DescriptorHeap *dsvHeap = nullptr;
};

graphics::RenderingContext *
createDx12RenderingContext(const RenderingContextCreationSettings &settings,
                           uint32_t width, uint32_t height);

inline bool executeCommandList(ID3D12CommandQueue *queue,
                               FrameCommand *command) {
  assert(command->isListOpen);
  HRESULT res = command->commandList->Close();
  assert(SUCCEEDED(res) && "Error closing command list");
  ID3D12CommandList *commandLists[] = {command->commandList};
  queue->ExecuteCommandLists(1, commandLists);
  bool succeded = SUCCEEDED(res);
  assert(succeded);
  command->isListOpen = false;
  return succeded;
}

inline HRESULT resetCommandList(FrameCommand *command) {

  assert(!command->isListOpen);
  const HRESULT res =
      command->commandList->Reset(command->commandAllocator, nullptr);
  assert(SUCCEEDED(res));
  command->isListOpen = true;
  return res;
}

class Dx12RenderingContext final : public RenderingContext {
public:
  explicit Dx12RenderingContext(
      const RenderingContextCreationSettings &settings, uint32_t width,
      uint32_t height);
  ~Dx12RenderingContext() = default;
  // private copy and assignment
  Dx12RenderingContext(const Dx12RenderingContext &) = delete;
  Dx12RenderingContext &operator=(const Dx12RenderingContext &) = delete;

  bool initializeGraphics() override;
  bool newFrame() override;
  bool dispatchFrame() override;
  bool resize(uint32_t width, uint32_t height) override;

  inline Dx12Resources *getResources() { return &m_resources; }
  inline IDXGIFactory6 *getFactory() const { return m_dxgiFactory; }

  void flushGlobalCommandQueue();

private:
  void flushCommandQueue(ID3D12CommandQueue *queue);

private:
  Dx12Resources m_resources{};
  ID3D12Debug *m_debugController = nullptr;
  IDXGIFactory6 *m_dxgiFactory = nullptr;
  Dx12Adapter *m_adapter = nullptr;

  uint32_t m_internalResourceIndex = 0;
  uint32_t m_internalCurrentFence = 0;
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

} // namespace cp::graphics::dx12