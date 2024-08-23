#include "DeferredRenderPass.h"

void DeferredRenderPass::Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_textureSampler = renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    GraphicsPipelineSpecs specs;
    specs.FormatCount = 2;
    specs.Formats[0] = TextureFormat::R11G11B10Float;
    specs.Formats[1] = TextureFormat::RGBA8SNorm;
    specs.DepthEnabled = true;
    specs.Depth = DepthOperation::Less;
    specs.DepthFormat = TextureFormat::R32Depth;
    specs.Cull = CullMode::Back;
    specs.Fill = FillMode::Solid;
    specs.TransparencyEnabled = false;
    ShaderCompiler::CompileShader("Shaders/SimpleVertex.hlsl", ShaderType::Vertex, specs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/DeferredGBufferPixel.hlsl", ShaderType::Pixel, specs.ShadersBytecodes[ShaderType::Pixel]);

    m_deferredPipeline = renderer->CreateGraphicsPipeline(specs);

    m_depthBuffer = renderer->CreateTexture(width, height, TextureFormat::R32Depth, TextureType::DepthTarget);
    renderer->CreateDepthView(m_depthBuffer);
    
    m_constantBuffer = renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_constantBuffer);

    m_GBuffer.m_albedoRenderTarget = renderer->CreateTexture(width, height, TextureFormat::R11G11B10Float, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_GBuffer.m_albedoRenderTarget);
    renderer->CreateShaderResourceView(m_GBuffer.m_albedoRenderTarget);

    m_GBuffer.m_normalRenderTarget = renderer->CreateTexture(width, height, TextureFormat::RGBA8SNorm, TextureType::RenderTarget);
    renderer->CreateRenderTargetView(m_GBuffer.m_normalRenderTarget);
    renderer->CreateShaderResourceView(m_GBuffer.m_normalRenderTarget);

    // TODO : resize render targets !
}

void DeferredRenderPass::Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems)
{
    auto view = camera.GetViewMatrix();
    auto proj = camera.GetProjMatrix();

    DirectX::XMMATRIX viewProj = view * proj;
    
    SceneConstantBuffer cbuf;
    cbuf.Time = globalPassData.ElapsedTime;
    cbuf.CameraPosition = camera.GetPosition();
    cbuf.Mode = globalPassData.ViewMode;
    DirectX::XMStoreFloat4x4(&cbuf.ViewProj, DirectX::XMMatrixTranspose(viewProj));
        
    void* data;
    m_constantBuffer->Map(0, 0, &data);
    memcpy(data, &cbuf, sizeof(SceneConstantBuffer));
    m_constantBuffer->Unmap(0, 0);
    
    auto commandList = renderer->GetCurrentCommandList();

    commandList->ImageBarrier(m_GBuffer.m_albedoRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ImageBarrier(m_GBuffer.m_normalRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList->ClearRenderTarget(m_GBuffer.m_albedoRenderTarget, 0.0f, 0.0f, 0.0f, 0.0f);
    commandList->ClearRenderTarget(m_GBuffer.m_normalRenderTarget, 0.0f, 0.0f, 0.0f, 0.0f);
    commandList->ClearDepthTarget(m_depthBuffer);

    commandList->BindRenderTargets({ m_GBuffer.m_albedoRenderTarget, m_GBuffer.m_normalRenderTarget }, m_depthBuffer);

    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_deferredPipeline);
    commandList->BindConstantBuffer(m_constantBuffer, 0);
    commandList->BindGraphicsSampler(m_textureSampler, 2);

    for(const auto renderItem : renderItems)
    {
        auto& material = renderItem->GetMaterial();

        if(material.HasAlbedo)
            commandList->BindGraphicsShaderResource(material.Albedo, 3);

        if(material.HasNormal)
            commandList->BindGraphicsShaderResource(material.Normal, 4);
            
        const auto primitives = renderItem->GetPrimitives();
        for(const auto& primitive : primitives)
        {
            commandList->BindConstantBuffer(primitive.m_objectConstantBuffer, 1);
            commandList->BindVertexBuffer(primitive.m_vertexBuffer);
            commandList->BindIndexBuffer(primitive.m_indicesBuffer);
            commandList->DrawIndexed(primitive.m_indexCount);
        }
    }

    commandList->ImageBarrier(m_GBuffer.m_albedoRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandList->ImageBarrier(m_GBuffer.m_normalRenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
}
