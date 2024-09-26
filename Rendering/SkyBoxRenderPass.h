﻿#pragma once
#include "RenderPass.h"
#include <wrl/client.h>

struct EnvironmentMaps
{
    std::shared_ptr<TextureCube> SkyBox;
    std::shared_ptr<TextureCube> DiffuseIrradianceMap;
    std::shared_ptr<TextureCube> PrefilterEnvMap;
    std::shared_ptr<Texture> BRDFLut;
};

class SkyBoxRenderPass : public RenderPass
{
public:
    void Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;
    void Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget) override;
    void OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height) override;

    EnvironmentMaps GetEnvironmentMaps() { return m_enviroMaps; }

private:
    std::shared_ptr<GraphicsPipeline> m_skyboxPipeline;
    std::shared_ptr<Sampler> m_textureSampler;
    std::shared_ptr<Buffer> m_constantBuffer;
    std::shared_ptr<Buffer> m_prefilterConstantBuffers[6];
    std::shared_ptr<RenderItem> m_sphereMesh;

    EnvironmentMaps m_enviroMaps;
};
