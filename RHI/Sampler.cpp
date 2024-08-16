#include "Sampler.h"

Sampler::Sampler(std::shared_ptr<Device> device, std::shared_ptr<DescriptorHeap> samplerHeap, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_FILTER filter)
{
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.AddressU = addressMode;
    samplerDesc.AddressV = samplerDesc.AddressU;
    samplerDesc.AddressW = samplerDesc.AddressV;
    samplerDesc.Filter = filter;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    m_descriptorHandle = samplerHeap->Allocate();
    device->GetDevice()->CreateSampler(&samplerDesc, m_descriptorHandle.CPU);
    m_heap = samplerHeap;
}

Sampler::~Sampler()
{
    m_heap->Free(m_descriptorHandle);
}
