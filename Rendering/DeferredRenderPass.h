#pragma once
#include "RenderPass.h"

struct GBuffer
{
    std::shared_ptr<Texture> AlbedoRenderTarget;
    std::shared_ptr<Texture> NormalRenderTarget;
    std::shared_ptr<Texture> WorldPositionRenderTarget;
    std::shared_ptr<Texture> DepthBuffer;
};

class DeferredRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) override;
    void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;

    GBuffer GetGBuffer() { return m_GBuffer; }

    void SetPBRDebugSettings(PBRDebugSettings debugSettings) { m_PBRDebugSettings = debugSettings; } // TODO remove this when PBR done

private:
    std::shared_ptr<GraphicsPipeline> m_deferredGeometryPipeline;
    std::shared_ptr<GraphicsPipeline> m_deferredDirLightPipeline;
    std::shared_ptr<GraphicsPipeline> m_deferredPointLightPipeline;
    std::shared_ptr<Buffer> m_sceneConstantBuffer;
    std::shared_ptr<Buffer> m_lightingConstantBuffer;
    std::shared_ptr<Sampler> m_textureSampler;
    std::shared_ptr<Buffer> m_screenQuadVertexBuffer;
    std::shared_ptr<RenderItem> m_pointLightMesh;
    std::vector<std::shared_ptr<Buffer>> m_lightsConstantBuffers;
    std::vector<std::shared_ptr<Buffer>> m_lightsInfosConstantBuffers;
    
    GBuffer m_GBuffer;

    // TODO remove this when PBR done
    PBRDebugSettings m_PBRDebugSettings = {};
    std::shared_ptr<Buffer> m_PBRDebugConstantBuffer;

    // TODO debug structured buffers
    // std::shared_ptr<Buffer> m_colorsStructuredBuffer;
};
