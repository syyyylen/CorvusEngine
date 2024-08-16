#pragma once
#include <Core.h>

#include "Allocator.h"
#include "DescriptorHeap.h"
#include "Device.h"

enum class TextureType
{
    ShaderResource,
    RenderTarget,
    DepthTarget,
    Storage,
    Copy
};

enum class TextureFormat
{
    RGBA8 = DXGI_FORMAT_R8G8B8A8_UNORM,
    RGBA32Float = DXGI_FORMAT_R32G32B32A32_FLOAT,
    RGBA16Float = DXGI_FORMAT_R16G16B16A16_FLOAT,
    R32Depth = DXGI_FORMAT_D32_FLOAT
};

class Texture 
{
public:
    Texture(std::shared_ptr<Device> device);
    Texture(std::shared_ptr<Device> device, std::shared_ptr<Allocator> allocator, uint32_t width, uint32_t height, TextureFormat format, TextureType type);
    ~Texture();

    void CreateRenderTarget(std::shared_ptr<DescriptorHeap> heap);
    void CreateDepthTarget(std::shared_ptr<DescriptorHeap> heap);
    void CreateShaderResource(std::shared_ptr<DescriptorHeap> heap);
    void CreateStorage(std::shared_ptr<DescriptorHeap> heap);

    void SetState(D3D12_RESOURCE_STATES state) { m_state = state; }
    D3D12_RESOURCE_STATES GetState() { return m_state; }
    GPUResource& GetResource() { return m_resource; }

    DescriptorHandle m_rtv;
    DescriptorHandle m_dsv;
    DescriptorHandle m_srvUav;

private:
    friend class SwapChain;

    std::shared_ptr<Device> m_device;
    TextureFormat m_format;
    D3D12_RESOURCE_STATES m_state;

    GPUResource m_resource;
    bool m_hasAlloc = false;
};