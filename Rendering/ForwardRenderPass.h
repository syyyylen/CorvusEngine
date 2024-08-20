#pragma once
#include "RenderPass.h"

class ForwardRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems) override;

private:
    std::shared_ptr<GraphicsPipeline> m_trianglePipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Buffer> m_lightsConstantBuffer;
    std::shared_ptr<Sampler> m_textureSampler;

    // TODO debug
    DirectX::XMFLOAT3 m_lightPosition = { 0.0f, 0.0f, -3.0f };
    DirectX::XMFLOAT4 m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float m_lightQuadraticAttenuation = 0.1f;
    float m_lightConstantAttenuation = 0.2f;
    float m_lightLinearAttenuation = 1.0f;
    // TODO debug
};
