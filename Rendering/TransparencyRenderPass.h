﻿#pragma once
#include "RenderPass.h"

class TransparencyRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData) override;
    void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;

private:
    std::shared_ptr<GraphicsPipeline> m_forwardTransparencyPipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Texture> m_depthBuffer;
    std::shared_ptr<Sampler> m_textureSampler;
};
