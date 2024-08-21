#pragma once
#include "RenderPass.h"

class TransparencyRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) override;

private:
    std::shared_ptr<GraphicsPipeline> m_forwardTransparencyPipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Sampler> m_textureSampler;
};
