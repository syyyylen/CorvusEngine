#include "DeferredRenderPass.h"

void DeferredRenderPass::Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_textureSampler = renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    GraphicsPipelineSpecs geomSpecs;
    geomSpecs.FormatCount = 4;
    geomSpecs.Formats[0] = TextureFormat::R11G11B10Float;
    geomSpecs.Formats[1] = TextureFormat::RGBA8SNorm;
    geomSpecs.Formats[2] = TextureFormat::R11G11B10Float;
    geomSpecs.Formats[3] = TextureFormat::R11G11B10Float;
    geomSpecs.DepthEnabled = true;
    geomSpecs.Depth = DepthOperation::Less;
    geomSpecs.DepthFormat = TextureFormat::R32Depth;
    geomSpecs.Cull = CullMode::Back;
    geomSpecs.Fill = FillMode::Solid;
    geomSpecs.BlendOperation = BlendOperation::None;
    ShaderCompiler::CompileShader("Shaders/SimpleVertex.hlsl", ShaderType::Vertex, geomSpecs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/DeferredGBufferPixel.hlsl", ShaderType::Pixel, geomSpecs.ShadersBytecodes[ShaderType::Pixel]);

    m_deferredGeometryPipeline = renderer->CreateGraphicsPipeline(geomSpecs);

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

    m_lightingConstantBuffer = renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_lightingConstantBuffer);

    OnResize(renderer, width, height);

    ScreenQuadVertex quadVerts[] =
    {
        { { -1.0f,1.0f, 0.0f,1.0f },{ 0.0f, 0.0f } },
        { { 1.0f, 1.0f, 0.0f,1.0f }, {1.0f,0.0f } },
        { { -1.0f, -1.0f, 0.0f,1.0f },{ 0.0f,1.0f } },
        { { 1.0f, -1.0f, 0.0f,1.0f },{ 1.0f,1.0f } }
    };

    m_screenQuadVertexBuffer = renderer->CreateBuffer(sizeof(ScreenQuadVertex) * 4, sizeof(ScreenQuadVertex), BufferType::Vertex, false);

    Uploader uploader = renderer->CreateUploader();
    uploader.CopyHostToDeviceLocal(quadVerts, sizeof(ScreenQuadVertex) * 4, m_screenQuadVertexBuffer);
    renderer->FlushUploader(uploader);

    m_pointLightMesh = std::make_shared<RenderItem>();
    m_pointLightMesh->ImportMesh(renderer, "Assets/sphere.gltf");
    m_instancedLightsInstanceDataTransformBuffer = renderer->CreateBuffer(sizeof(InstanceData) * MAX_LIGHTS, sizeof(InstanceData), BufferType::Structured, false);
    m_instancedLightsInstanceDataInfoBuffer = renderer->CreateBuffer(sizeof(PointLight) * MAX_LIGHTS, sizeof(PointLight), BufferType::Structured, false);
}

void DeferredRenderPass::OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_GBuffer.AlbedoRenderTarget.reset();
    m_GBuffer.NormalRenderTarget.reset();
    m_GBuffer.WorldPositionRenderTarget.reset();
    m_GBuffer.MetallicRoughnessRenderTarget.reset();
    m_GBuffer.DepthBuffer.reset();
    m_renderTexture.reset();
    
    m_GBuffer.DepthBuffer = renderer->CreateTexture(width, height, TextureFormat::R32Depth, TextureType::DepthTarget);
    renderer->CreateDepthView(m_GBuffer.DepthBuffer);
    m_GBuffer.DepthBuffer->SetFormat(TextureFormat::R32Float);
    renderer->CreateShaderResourceView(m_GBuffer.DepthBuffer);
    m_GBuffer.DepthBuffer->SetFormat(TextureFormat::R32Depth);
    
    m_GBuffer.AlbedoRenderTarget = renderer->CreateTexture(width, height, TextureFormat::R11G11B10Float, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_GBuffer.AlbedoRenderTarget);
    renderer->CreateShaderResourceView(m_GBuffer.AlbedoRenderTarget);

    m_GBuffer.NormalRenderTarget = renderer->CreateTexture(width, height, TextureFormat::RGBA8SNorm, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_GBuffer.NormalRenderTarget);
    renderer->CreateShaderResourceView(m_GBuffer.NormalRenderTarget);

    m_GBuffer.WorldPositionRenderTarget = renderer->CreateTexture(width, height, TextureFormat::R11G11B10Float, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_GBuffer.WorldPositionRenderTarget);
    renderer->CreateShaderResourceView(m_GBuffer.WorldPositionRenderTarget);

    m_GBuffer.MetallicRoughnessRenderTarget = renderer->CreateTexture(width, height, TextureFormat::R11G11B10Float, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_GBuffer.MetallicRoughnessRenderTarget);
    renderer->CreateShaderResourceView(m_GBuffer.MetallicRoughnessRenderTarget);

    m_renderTexture = renderer->CreateTexture(width, height, TextureFormat::RGBA8, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_renderTexture);
    renderer->CreateShaderResourceView(m_renderTexture);
}

void DeferredRenderPass::Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems)
{
    // ------------------------------------------------------------- Geometry Pass (GBuffer --------------------------------------------------------------------
    auto view = camera.GetViewMatrix();
    auto proj = camera.GetProjMatrix();

    DirectX::XMMATRIX viewProj = view * proj;
    
    SceneConstantBuffer cbuf;
    cbuf.Time = globalPassData.ElapsedTime;
    cbuf.CameraPosition = camera.GetPosition();
    cbuf.Mode = globalPassData.ViewMode;
    cbuf.DirLightDirection = globalPassData.DirectionalInfo.Direction;
    cbuf.DirLightIntensity = globalPassData.DirectionalInfo.Intensity;
    DirectX::XMStoreFloat4x4(&cbuf.ViewProj, viewProj);
        
    void* data;
    m_sceneConstantBuffer->Map(0, 0, &data);
    memcpy(data, &cbuf, sizeof(SceneConstantBuffer));
    m_sceneConstantBuffer->Unmap(0, 0);
    
    auto commandList = renderer->GetCurrentCommandList();

    commandList->SetViewport(0, 0, globalPassData.viewportSizeX, globalPassData.viewportSizeY);

    std::unordered_map<std::shared_ptr<Texture>, D3D12_RESOURCE_STATES> renderTargetsBatchedBarriers;
    renderTargetsBatchedBarriers.emplace(m_GBuffer.AlbedoRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderTargetsBatchedBarriers.emplace(m_GBuffer.NormalRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderTargetsBatchedBarriers.emplace(m_GBuffer.WorldPositionRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderTargetsBatchedBarriers.emplace(m_GBuffer.MetallicRoughnessRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderTargetsBatchedBarriers.emplace(m_GBuffer.DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    commandList->ImageBarrier(renderTargetsBatchedBarriers);

    commandList->ClearRenderTarget(m_GBuffer.AlbedoRenderTarget, 0.0f, 0.0f, 0.0f, 1.0f);
    commandList->ClearRenderTarget(m_GBuffer.NormalRenderTarget, 0.0f, 0.0f, 0.0f, 1.0f);
    commandList->ClearRenderTarget(m_GBuffer.WorldPositionRenderTarget, 100.0f, 100.0f, 100.0f, 1.0f);
    commandList->ClearRenderTarget(m_GBuffer.MetallicRoughnessRenderTarget, 0.0f, 0.0f, 0.0f, 1.0f);
    commandList->ClearDepthTarget(m_GBuffer.DepthBuffer);

    commandList->BindRenderTargets({  m_GBuffer.AlbedoRenderTarget,
                                        m_GBuffer.NormalRenderTarget,
                                        m_GBuffer.WorldPositionRenderTarget,
                                        m_GBuffer.MetallicRoughnessRenderTarget },
                                        m_GBuffer.DepthBuffer);

    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_deferredGeometryPipeline);
    commandList->BindConstantBuffer(m_sceneConstantBuffer, 0);
    commandList->BindGraphicsSampler(m_textureSampler, 1);

    for(const auto renderItem : renderItems)
    {
        auto& material = renderItem->GetMaterial();

        std::vector<InstanceData> instancesData;
        auto instancesTransforms = renderItem->m_transforms;
        for(auto instanceTransform : instancesTransforms)
        {
            InstanceData instanceData;
            instanceData.WorldMat = instanceTransform;
            instanceData.HasAlbedo = material.HasAlbedo;
            instanceData.HasNormalMap = material.HasNormal;
            instanceData.HasMetallicRoughness = material.HasMetallicRoughness;
            instancesData.emplace_back(instanceData);
        }

        void* dt;
        renderItem->m_instancesDataBuffer->Map(0, 0, &dt);
        memcpy(dt, instancesData.data(), sizeof(InstanceData) * renderItem->m_transforms.size());
        renderItem->m_instancesDataBuffer->Unmap(0, 0);

        commandList->SetGraphicsShaderResource(renderItem->m_instancesDataBuffer, 5);

        if(material.HasAlbedo)
            commandList->BindGraphicsShaderResource(material.Albedo, 2);

        if(material.HasNormal)
            commandList->BindGraphicsShaderResource(material.Normal, 3);

        if(material.HasMetallicRoughness)
            commandList->BindGraphicsShaderResource(material.MetallicRoughness, 4);
            
        const auto primitives = renderItem->GetPrimitives();
        for(const auto& primitive : primitives)
        {
            commandList->BindVertexBuffer(primitive.m_vertexBuffer);
            commandList->BindIndexBuffer(primitive.m_indicesBuffer);
            commandList->DrawIndexed(primitive.m_indexCount, renderItem->m_transforms.size());
        }
    }

    // ------------------------------------------------------------- Lighting Pass (directional) --------------------------------------------------------------------

    std::unordered_map<std::shared_ptr<Texture>, D3D12_RESOURCE_STATES> readBatchedBarriers;
    readBatchedBarriers.emplace(m_GBuffer.AlbedoRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
    readBatchedBarriers.emplace(m_GBuffer.NormalRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
    readBatchedBarriers.emplace(m_GBuffer.WorldPositionRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
    readBatchedBarriers.emplace(m_GBuffer.MetallicRoughnessRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
    readBatchedBarriers.emplace(m_GBuffer.DepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandList->ImageBarrier(readBatchedBarriers);

    auto invViewProj = camera.GetInvViewProjMatrix();
    DirectX::XMStoreFloat4x4(&cbuf.ViewProj, invViewProj);
        
    void* data2;
    m_lightingConstantBuffer->Map(0, 0, &data2);
    memcpy(data2, &cbuf, sizeof(SceneConstantBuffer));
    m_lightingConstantBuffer->Unmap(0, 0);

    commandList->ImageBarrier(m_renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->BindRenderTargets({ m_renderTexture }, nullptr);
    
    commandList->SetTopology(Topology::TriangleStrip);
    commandList->BindGraphicsPipeline(m_deferredDirLightPipeline);
    commandList->BindConstantBuffer(m_lightingConstantBuffer, 0);
    commandList->BindGraphicsSampler(m_textureSampler, 1);
    commandList->BindGraphicsShaderResource(m_GBuffer.AlbedoRenderTarget, 2);
    commandList->BindGraphicsShaderResource(m_GBuffer.NormalRenderTarget, 3);
    commandList->BindGraphicsShaderResource(m_GBuffer.WorldPositionRenderTarget, 4);
    commandList->BindGraphicsShaderResource(m_GBuffer.MetallicRoughnessRenderTarget, 5);
    commandList->BindGraphicsShaderResource(m_GBuffer.DepthBuffer, 6);
    commandList->BindVertexBuffer(m_screenQuadVertexBuffer);
    commandList->Draw(4);

    // ------------------------------------------------------------- Lights Volumes --------------------------------------------------------------------

    if(globalPassData.PointLights.empty() || globalPassData.ViewMode > 0)
        return;

    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_deferredPointLightPipeline);
    commandList->BindConstantBuffer(m_sceneConstantBuffer, 0);
    commandList->BindGraphicsShaderResource(m_GBuffer.AlbedoRenderTarget, 2);
    commandList->BindGraphicsShaderResource(m_GBuffer.NormalRenderTarget, 3);
    commandList->BindGraphicsShaderResource(m_GBuffer.WorldPositionRenderTarget, 4);
    commandList->BindGraphicsShaderResource(m_GBuffer.MetallicRoughnessRenderTarget, 5);

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
    
    commandList->ImageBarrier(m_renderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
}
