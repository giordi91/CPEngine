#pragma once
#include <d3d12.h>

struct DescriptorPair {
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
  D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
//TODO fix this
#if SE_DEBUG
  DescriptorType type;
#endif
};

#if DXR_ENABLED
typedef ID3D12Device5 D3D12DeviceType;
#else
typedef ID3D12Device3 D3D12DeviceType;
#endif