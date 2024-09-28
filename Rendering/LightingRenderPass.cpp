﻿#include "LightingRenderPass.h"

void LightingRenderPass::Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_textureSampler = renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    m_comparisonSampler = renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    GraphicsPipelineSpecs dirLightSpecs;
    dirLightSpecs.FormatCount = 1;
    dirLightSpecs.Formats[0] = TextureFormat::RGBA8;
    dirLightSpecs.BlendOperation = BlendOperation::None;
    dirLightSpecs.DepthEnabled = false;
    dirLightSpecs.Cull = CullMode::Back;
    dirLightSpecs.Fill = FillMode::Solid;
    ShaderCompiler::CompileShader("Shaders/ScreenQuadVertex.hlsl", ShaderType::Vertex, dirLightSpecs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/DeferredLightingPixel.hlsl", ShaderType::Pixel, dirLightSpecs.ShadersBytecodes[ShaderType::Pixel]);

    m_deferredDirLightPipeline = renderer->CreateGraphicsPipeline(dirLightSpecs);

    GraphicsPipelineSpecs pointLightSpecs;
    pointLightSpecs.FormatCount = 1;
    pointLightSpecs.Formats[0] = TextureFormat::RGBA8;
    pointLightSpecs.BlendOperation = BlendOperation::Additive;
    pointLightSpecs.Cull = CullMode::Back;
    pointLightSpecs.Fill = FillMode::Solid;
    pointLightSpecs.DepthEnabled = false;
    ShaderCompiler::CompileShader("Shaders/DeferredPointLightVertex.hlsl", ShaderType::Vertex, pointLightSpecs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/DeferredPointLightPixel.hlsl", ShaderType::Pixel, pointLightSpecs.ShadersBytecodes[ShaderType::Pixel]);

    m_deferredPointLightPipeline = renderer->CreateGraphicsPipeline(pointLightSpecs);

    m_sceneConstantBuffer = renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_sceneConstantBuffer);

    OnResize(renderer, width, height);

    m_pointLightMesh = std::make_shared<RenderItem>();
    m_pointLightMesh->ImportMesh(renderer, "Assets/sphere.gltf");
    m_instancedLightsInstanceDataTransformBuffer = renderer->CreateBuffer(sizeof(InstanceData) * MAX_LIGHTS, sizeof(InstanceData), BufferType::Structured, false);
    m_instancedLightsInstanceDataInfoBuffer = renderer->CreateBuffer(sizeof(PointLight) * MAX_LIGHTS, sizeof(PointLight), BufferType::Structured, false);
}

void LightingRenderPass::OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
}

void LightingRenderPass::Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget)
{
    auto view = camera.GetViewMatrix();
    auto proj = camera.GetProjMatrix();
    auto invViewProj = camera.GetInvViewProjMatrix();

    DirectX::XMMATRIX viewProj = view * proj;
    
    SceneConstantBuffer cbuf;
    cbuf.Time = globalPassData.ElapsedTime;
    cbuf.CameraPosition = camera.GetPosition();
    cbuf.Mode = globalPassData.ViewMode;
    cbuf.DirLightDirection = globalPassData.DirectionalInfo.Direction;
    cbuf.DirLightIntensity = globalPassData.DirectionalInfo.Intensity;
    cbuf.ScreenDimensions[0] = globalPassData.ViewportSizeX;
    cbuf.ScreenDimensions[1] = globalPassData.ViewportSizeY;
    DirectX::XMStoreFloat4x4(&cbuf.ViewProj, viewProj);
    DirectX::XMStoreFloat4x4(&cbuf.InvViewProj, invViewProj);
    cbuf.ShadowTransform = globalPassData.ShadowMap.ShadowTransform;
    cbuf.ShadowEnabled = globalPassData.EnableShadows;

    auto GBuffer = globalPassData.GBuffer;
        
    void* data;
    m_sceneConstantBuffer->Map(0, 0, &data);
    memcpy(data, &cbuf, sizeof(SceneConstantBuffer));
    m_sceneConstantBuffer->Unmap(0, 0);
    
    auto commandList = renderer->GetCurrentCommandList();
    commandList->SetViewport(0, 0, globalPassData.ViewportSizeX, globalPassData.ViewportSizeY);

    // ------------------------------------------------------------- Lighting Pass (directional) --------------------------------------------------------------------

    commandList->ImageBarrier(renderTarget.RenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->BindRenderTargets({ renderTarget.RenderTexture }, nullptr);
    
    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_deferredDirLightPipeline);
    commandList->BindGraphicsConstantBuffer(m_sceneConstantBuffer, 0);
    commandList->BindGraphicsSampler(m_textureSampler, 1);
    commandList->BindGraphicsShaderResource(GBuffer.AlbedoRenderTarget, 2);
    commandList->BindGraphicsShaderResource(GBuffer.NormalRenderTarget, 3);
    commandList->BindGraphicsShaderResource(GBuffer.MetallicRoughnessRenderTarget, 4);
    commandList->BindGraphicsShaderResource(GBuffer.DepthBuffer, 5);
    commandList->BindGraphicsShaderResource(globalPassData.IrradianceMap, 6);
    commandList->BindGraphicsShaderResource(globalPassData.PrefilterEnvMap, 7);
    if(globalPassData.EnableShadows)
    {
        commandList->BindGraphicsShaderResource(globalPassData.ShadowMap.DepthBuffer, 8);
        commandList->BindGraphicsSampler(m_comparisonSampler, 9);
    }
    // commandList->BindGraphicsShaderResource(globalPassData.BRDFLut, 8);
    commandList->Draw(6);

    // ------------------------------------------------------------- Lights Volumes --------------------------------------------------------------------

    if(globalPassData.PointLights.empty() /* || globalPassData.ViewMode > 0 */)
        return;

    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_deferredPointLightPipeline);
    commandList->BindGraphicsConstantBuffer(m_sceneConstantBuffer, 0);
    commandList->BindGraphicsShaderResource(GBuffer.AlbedoRenderTarget, 2);
    commandList->BindGraphicsShaderResource(GBuffer.NormalRenderTarget, 3);
    commandList->BindGraphicsShaderResource(GBuffer.MetallicRoughnessRenderTarget, 4);
    commandList->BindGraphicsShaderResource(GBuffer.DepthBuffer, 5);

    std::vector<InstanceData> instancesData;
    for(int i = 0; i < globalPassData.PointLights.size(); i++)
    {
        float radius = globalPassData.PointLights[i].Radius;

        InstanceData instanceData;
        DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
        mat *= DirectX::XMMatrixScaling(radius, radius, radius);
        mat *= DirectX::XMMatrixTranslation(globalPassData.PointLights[i].Position.x, globalPassData.PointLights[i].Position.y, globalPassData.PointLights[i].Position.z);
        DirectX::XMStoreFloat4x4(&instanceData.WorldMat, mat);
        instancesData.emplace_back(instanceData);
    }

    void* dt;
    m_instancedLightsInstanceDataTransformBuffer->Map(0, 0, &dt);
    memcpy(dt, instancesData.data(), sizeof(InstanceData) * globalPassData.PointLights.size());
    m_instancedLightsInstanceDataTransformBuffer->Unmap(0, 0);

    void* dt2;
    m_instancedLightsInstanceDataInfoBuffer->Map(0, 0, &dt2);
    memcpy(dt2, globalPassData.PointLights.data(), sizeof(PointLight) * globalPassData.PointLights.size());
    m_instancedLightsInstanceDataInfoBuffer->Unmap(0, 0);

    commandList->SetGraphicsShaderResource(m_instancedLightsInstanceDataTransformBuffer, 1);
    commandList->SetGraphicsShaderResource(m_instancedLightsInstanceDataInfoBuffer, 6);
    commandList->BindVertexBuffer(m_pointLightMesh->GetPrimitives()[0].m_vertexBuffer);
    commandList->BindIndexBuffer(m_pointLightMesh->GetPrimitives()[0].m_indicesBuffer);
    commandList->DrawIndexed(m_pointLightMesh->GetPrimitives()[0].m_indexCount, globalPassData.PointLights.size());
    
    commandList->ImageBarrier(renderTarget.RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
}
