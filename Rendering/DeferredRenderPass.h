﻿#pragma once
#include "RenderPass.h"

struct GBuffer
{
    std::shared_ptr<Texture> m_albedoRenderTarget;
    std::shared_ptr<Texture> m_normalRenderTarget;
};

class DeferredRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) override;

    GBuffer GetGBuffer() { return m_GBuffer; }

private:
    std::shared_ptr<GraphicsPipeline> m_deferredPipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Sampler> m_textureSampler;
    std::shared_ptr<Texture> m_depthBuffer;
    GBuffer m_GBuffer;
};
