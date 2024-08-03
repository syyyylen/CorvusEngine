#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t size)
    : m_device(device), m_type(type), m_heapSize(size), m_isShaderVisible(false)
{
    m_handlesTable = std::vector<bool>(size, false);
    
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
}

DescriptorHeap::~DescriptorHeap()
{
    m_handlesTable.clear();
    m_heap->Release();
}

DescriptorHandle DescriptorHeap::Allocate()
{
    int index = -1;
    for(int i = 0; i < m_heapSize; i++)
    {
        if(m_handlesTable[i] == false)
        {
            m_handlesTable[i] = true;
            index = i;
            break;
        }
    }

    if(index == -1)
    {
        LOG(Error, "DescriptorHeap : failed to create descriptor handle !");
        return DescriptorHandle();
    }

    DescriptorHandle descriptorHandle = {};
    descriptorHandle.HeapIdx = index;
    descriptorHandle.CPU = m_heap->GetCPUDescriptorHandleForHeapStart();
    descriptorHandle.CPU.ptr += index * m_incrementSize;

    if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || m_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
    {
        descriptorHandle.GPU = m_heap->GetGPUDescriptorHandleForHeapStart();
        descriptorHandle.GPU.ptr += index * m_incrementSize;
    }

    return descriptorHandle;
}

void DescriptorHeap::Free(DescriptorHandle& handle)
{
    return; // TODO this crashes, to fix
    if(!handle.IsValid())
        return;

    m_handlesTable[handle.HeapIdx] = false;
    handle.CPU.ptr = 0;
}