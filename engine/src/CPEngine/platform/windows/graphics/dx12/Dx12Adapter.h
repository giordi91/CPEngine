#pragma once
#include <dxgi1_6.h>

// forward
namespace cp::graphics{
namespace dx12 {
enum class ADAPTER_VENDOR { NVIDIA, AMD, INTEL, ANY };

enum class ADAPTER_FEATURE { DXR = 2, ANY = 4 };

class Dx12Adapter {
public:
  Dx12Adapter() = default;
  ~Dx12Adapter();
  inline void setVendor(const ADAPTER_VENDOR vendor) { m_vendor = vendor; }
  inline void setFeature(const ADAPTER_FEATURE feature) { m_feature = feature; }
  inline ADAPTER_VENDOR getVendor() const { return m_vendor; }
  inline ADAPTER_FEATURE getFeature() const { return m_feature; }

  bool findBestDx12Adapter(IDXGIFactory4 *dxgiFactory, bool verbose = false);
  inline IDXGIAdapter3 *getDx12Adapter() const { return m_adapter; }

private:
  ADAPTER_VENDOR m_vendor = ADAPTER_VENDOR::NVIDIA;
  ADAPTER_FEATURE m_feature = ADAPTER_FEATURE::ANY;
  IDXGIAdapter3 *m_adapter = nullptr;
};
} // namespace dx12
} // namespace SirEngine
