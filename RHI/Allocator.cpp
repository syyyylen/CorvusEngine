#include "Allocator.h"

Allocator::Allocator(std::shared_ptr<Device> device)
{
    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.pAdapter = device->GetAdapter();
    desc.pDevice = device->GetDevice();

    HRESULT hr = D3D12MA::CreateAllocator(&desc, &m_allocator);
    if(FAILED(hr))
    {
        LOG(Error, "Allocator : failed to create allocator !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    LOG(Debug, "Allocator created !");
}

Allocator::~Allocator()
{
    m_allocator->Release();
}

GPUResource Allocator::Allocate(D3D12MA::ALLOCATION_DESC* allocDesc, D3D12_RESOURCE_DESC* resDesc, D3D12_RESOURCE_STATES states)
{
    GPUResource resource = {};

    HRESULT hr = m_allocator->CreateResource(allocDesc, resDesc, states, nullptr, &resource.Allocation, IID_PPV_ARGS(&resource.Resource));
    if(FAILED(hr))
    {
        LOG(Error, "Allocator : failed to allocate !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }
    return resource;
}