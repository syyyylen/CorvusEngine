#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t size)
    : m_device(device), m_type(type), m_heapSize(size), m_isShaderVisible(false)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = m_type;
    desc.NumDescriptors = m_heapSize;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || m_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
    {
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        m_isShaderVisible = true;
    }

    HRESULT hr = m_device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
    if(FAILED(hr))
    {
        LOG(Error, "DescriptorHeap : failed to create descriptor heap !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    m_incrementSize = m_device->GetDevice()->GetDescriptorHandleIncrementSize(m_type);

    LOG(Debug, "DescriptorHeap : allocated a descriptor heap of size : " + std::to_string(size));

    m_freeHandles = std::move(std::make_unique<uint32_t[]>(m_heapSize));
    m_size = 0;

    for(uint32_t i = 0; i < m_heapSize; i++)
        m_freeHandles[i] = i;
}

DescriptorHeap::~DescriptorHeap()
{
    m_freeHandles.release();
    m_heap->Release();
}

DescriptorHandle DescriptorHeap::Allocate()
{
    uint32_t idx = m_freeHandles[m_size];
    uint32_t offset = idx * m_incrementSize;
    m_size++;

    DescriptorHandle handle = {};
    handle.HeapIdx = idx;

    auto CPUstartPtr = m_heap->GetCPUDescriptorHandleForHeapStart();
    handle.CPU.ptr = CPUstartPtr.ptr + offset;
    if(m_isShaderVisible)
    {
        auto GPUstartPtr = m_heap->GetGPUDescriptorHandleForHeapStart();
        handle.GPU.ptr = GPUstartPtr.ptr + offset;
    }

    return handle;
}

void DescriptorHeap::Free(DescriptorHandle& handle)
{
    if(!handle.IsValid())
        return;

    handle = {};
}