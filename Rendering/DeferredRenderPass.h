#pragma once
#include "RenderPass.h"

struct GBuffer
{
    std::shared_ptr<Texture> AlbedoRenderTarget;
    std::shared_ptr<Texture> NormalRenderTarget;
    std::shared_ptr<Texture> MetallicRoughnessRenderTarget;
    std::shared_ptr<Texture> DepthBuffer;
};

class DeferredRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) override;
    void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    std::shared_ptr<Texture> GetRenderTexture() override { return m_renderTexture; }

    GBuffer GetGBuffer() { return m_GBuffer; }

private:
    std::shared_ptr<GraphicsPipeline> m_deferredGeometryPipeline;
    std::shared_ptr<GraphicsPipeline> m_deferredDirLightPipeline;
    std::shared_ptr<GraphicsPipeline> m_deferredPointLightPipeline;
    std::shared_ptr<Buffer> m_sceneConstantBuffer;
    std::shared_ptr<Sampler> m_textureSampler;
    std::shared_ptr<RenderItem> m_pointLightMesh;
    std::shared_ptr<Buffer> m_instancedLightsInstanceDataTransformBuffer;
    std::shared_ptr<Buffer> m_instancedLightsInstanceDataInfoBuffer;
    std::shared_ptr<Texture> m_renderTexture;
    
    GBuffer m_GBuffer;
};
