#pragma once
#include "Device.h"
#include <Core.h>
#include "D3D12MA/D3D12MemAlloc.h"

struct GPUResource
{
    ID3D12Resource* Resource;
    D3D12MA::Allocation* Allocation;
};

class Allocator 
{
public:
    Allocator(std::shared_ptr<Device> device);
    ~Allocator();

    GPUResource Allocate(D3D12MA::ALLOCATION_DESC* allocDesc, D3D12_RESOURCE_DESC* resDesc, D3D12_RESOURCE_STATES states);

    D3D12MA::Allocator* GetAllocator() { return m_allocator; }

private:
    D3D12MA::Allocator* m_allocator;
};