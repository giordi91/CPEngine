
#if CP_WINDOWS_PLATFORM

#include "CPEngine/platform/windows/graphics/dx12/Dx12Adapter.h"
#include "CPEngine/core/core.h"
#include "CPEngine/core/logging.h"
#include <d3d12.h>

namespace cp::graphics::dx12 {

Dx12Adapter::~Dx12Adapter() {
  if (m_adapter != nullptr) {
    m_adapter->Release();
  }
}
bool Dx12Adapter::findBestDx12Adapter(IDXGIFactory4 *dxgiFactory,
                                      bool verbose) {

  IDXGIAdapter1 *curDx12Adapter;
  size_t adapterMemory = 0;
  IDXGIAdapter3 *dxgiDx12Adapter = nullptr;
  // lets loop the adapters
  for (UINT adapterIdx = 0;
       dxgiFactory->EnumAdapters1(adapterIdx, &curDx12Adapter) == S_OK;
       ++adapterIdx) {

    IDXGIAdapter3 *adapter = (IDXGIAdapter3 *)curDx12Adapter;
    if (adapter == nullptr) {
      break;
    }

    // inspecting the description
    DXGI_ADAPTER_DESC desc;
    if (SUCCEEDED(adapter->GetDesc(&desc))) {
      bool isDXR = false;
      bool requiresDXR = m_feature == ADAPTER_FEATURE::DXR;
      // for now only checking DXR assuming Nvidia
      if (requiresDXR) {
        isDXR = (wcsstr(desc.Description, L"RTX") != 0);
      }

      // checking for Microsoft software adapter, we want to skip it
      bool isSoftwareVendor = desc.VendorId == 0x1414;
      bool isSoftwareId = desc.DeviceId == 0x8c;
      bool isSoftware = isSoftwareVendor & isSoftwareId;
      if (!((isSoftware) & (isDXR & requiresDXR))) {
        // then we just prioritize memory size, in the future we
        // might want to use also other metrics
        if (desc.DedicatedVideoMemory > adapterMemory) {
          dxgiDx12Adapter = adapter;
          adapterMemory = desc.DedicatedVideoMemory;
        }
      }
    }
  }
  m_adapter = dxgiDx12Adapter;

  return dxgiDx12Adapter != nullptr;
} // namespace dx12
} // namespace cp::graphics::dx12
#endif
