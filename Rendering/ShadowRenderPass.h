#pragma once
#include "RenderPass.h"

struct ShadowMap
{
    std::shared_ptr<Texture> DepthBuffer;
};

class ShadowRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget) override;
    void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;

    ShadowMap GetShadowMap() { return m_shadowMap; }

private:
    std::shared_ptr<GraphicsPipeline> m_shadowPipeline;
    std::shared_ptr<Buffer> m_constantBuffer;
    ShadowMap m_shadowMap;

    DirectX::XMFLOAT4X4 m_shadowTransform = {};

    int m_shadowMapWidth = 1920;
    int m_shadowMapHeight = 1080;
};
