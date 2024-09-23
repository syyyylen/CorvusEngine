#pragma once
#include "RenderPass.h"
#include <wrl/client.h>

// TODO remove this
struct CubeMap
{
    Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
    DescriptorHandle Handle;
};

class SkyBoxRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget) override;
    void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;

    CubeMap m_cubeMap = {};

private:
    std::shared_ptr<GraphicsPipeline> m_skyboxPipeline;
    std::shared_ptr<Sampler> m_textureSampler;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<RenderItem> m_sphereMesh;
};
