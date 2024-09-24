#include "TextureCube.h"
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

    auto ShaderHeap = heaps.ShaderHeap;
    m_srv = ShaderHeap->Allocate();
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = m_resourceComPtr->GetDesc().MipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format = m_resourceComPtr->GetDesc().Format;
    device->GetDevice()->CreateShaderResourceView(m_resourceComPtr.Get(), &srvDesc, m_srv.CPU);
}
