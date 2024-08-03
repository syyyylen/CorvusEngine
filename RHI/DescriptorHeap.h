#pragma once

#include "Device.h"
#include <Core.h>

struct DescriptorHandle
{
    D3D12_CPU_DESCRIPTOR_HANDLE CPU;
    D3D12_GPU_DESCRIPTOR_HANDLE GPU;

    uint32_t HeapIdx;

    bool IsValid() { return CPU.ptr != 0; }
    bool IsShaderVisible() { return GPU.ptr != 0; }
};

class DescriptorHeap 
{
public:
    DescriptorHeap(std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t size);
    ~DescriptorHeap();

    ID3D12DescriptorHeap* GetHeap() { return m_heap; }
    DescriptorHandle Allocate();
    void Free(DescriptorHandle& handle);

private:
    std::shared_ptr<Device> m_device;
    ID3D12DescriptorHeap* m_heap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    bool m_isShaderVisible;
    int m_incrementSize;
    int m_heapSize;

    std::vector<bool> m_handlesTable;
};