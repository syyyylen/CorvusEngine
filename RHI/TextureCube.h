#pragma once
#include <Core.h>
#include <wrl/client.h>
#include "Allocator.h"
#include "DescriptorHeap.h"
#include "Device.h"

class CommandList;

class TextureCube
{
public:
    TextureCube(std::shared_ptr<Device> device, std::shared_ptr<CommandList> cmdList, const std::wstring& filePath, Heaps& heaps);
    DescriptorHandle& GetSrvHandle() { return m_srv; }

private:
    friend class CommandList;
    DescriptorHandle m_srv;
    
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resourceComPtr;
};
