#include "ForwardRenderPass.h"

void ForwardRenderPass::Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_textureSampler = renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    GraphicsPipelineSpecs specs;
    specs.FormatCount = 1;
    specs.Formats[0] = TextureFormat::RGBA8;
    specs.DepthEnabled = true;
    specs.Depth = DepthOperation::Less;
    specs.DepthFormat = TextureFormat::R32Depth;
    specs.Cull = CullMode::Back;
    specs.Fill = FillMode::Solid;
    specs.BlendOperation = BlendOperation::None;
    ShaderCompiler::CompileShader("Shaders/SimpleVertex.hlsl", ShaderType::Vertex, specs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/SimplePixel.hlsl", ShaderType::Pixel, specs.ShadersBytecodes[ShaderType::Pixel]);

    m_forwardPipeline = renderer->CreateGraphicsPipeline(specs);
    
    m_constantBuffer = renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_constantBuffer);

    m_lightsConstantBuffer = renderer->CreateBuffer(256 * 2, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_lightsConstantBuffer);
}

void ForwardRenderPass::OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
}

void ForwardRenderPass::Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<std::shared_ptr<RenderItem>>& renderItems)
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

    PointLightsConstantBuffer lightsCbuf;
    for(int i = 0; i < globalPassData.PointLights.size(); i++)
    {
        if(i < MAX_LIGHTS)
            lightsCbuf.PointLights[i] = globalPassData.PointLights[i];
    }

    void* data2;
    m_lightsConstantBuffer->Map(0, 0, &data2);
    memcpy(data2, &lightsCbuf, sizeof(PointLightsConstantBuffer));
    m_lightsConstantBuffer->Unmap(0, 0);

    auto commandList = renderer->GetCurrentCommandList();
    
    commandList->SetTopology(Topology::TriangleList);

    commandList->BindGraphicsPipeline(m_forwardPipeline);
        
    commandList->BindConstantBuffer(m_constantBuffer, 0);
    commandList->BindConstantBuffer(m_lightsConstantBuffer, 5);
        
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
}