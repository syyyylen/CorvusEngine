﻿#include "TextureCube.h"
#include "CommandList.h"
#include "DDSTextureLoader/DDSTextureLoader.h"

TextureCube::TextureCube(std::shared_ptr<Device> device, std::shared_ptr<CommandList> cmdList, const std::wstring& filePath, Heaps& heaps)
{
    HRESULT hr = DirectX::CreateDDSTextureFromFile12(device->GetDevice(), cmdList->GetCommandList(), filePath.c_str(),m_resourceComPtr, uploadHeap);
    if(FAILED(hr))
    {
        LOG(Error, "failed to create dds texture !!!");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    m_width = m_resourceComPtr->GetDesc().Width;
    m_height = m_resourceComPtr->GetDesc().Height;

    auto shaderHeap = heaps.ShaderHeap;
    m_srv = shaderHeap->Allocate();
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = m_resourceComPtr->GetDesc().MipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format = m_resourceComPtr->GetDesc().Format;
    device->GetDevice()->CreateShaderResourceView(m_resourceComPtr.Get(), &srvDesc, m_srv.CPU);

    m_resource.Resource = m_resourceComPtr.Get();
}

TextureCube::TextureCube(std::shared_ptr<Device> device, std::shared_ptr<Allocator> allocator, uint32_t width, uint32_t height, TextureFormat format, Heaps& heaps)
{
    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.DepthOrArraySize = 6;
    resourceDesc.MipLevels = 5;
    resourceDesc.Format = DXGI_FORMAT(format);
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    m_state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    m_resource = allocator->Allocate(&allocDesc, &resourceDesc, m_state);
    m_hasAlloc = true;

    auto shaderHeap = heaps.ShaderHeap;
    m_srv = shaderHeap->Allocate();
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 5;
    srvDesc.Format = (DXGI_FORMAT)format;
    device->GetDevice()->CreateShaderResourceView(m_resource.Resource, &srvDesc, m_srv.CPU);

    for(int i = 0; i < 5; i++)
    {
        m_uavs[i] = shaderHeap->Allocate();

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice = i;
        uavDesc.Texture2DArray.ArraySize = 6;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Format = DXGI_FORMAT(format);

        device->GetDevice()->CreateUnorderedAccessView(m_resource.Resource, nullptr, &uavDesc, m_uavs[i].CPU);
    }
}

TextureCube::~TextureCube()
{
    if(m_hasAlloc)
        m_resource.Allocation->Release();
}
