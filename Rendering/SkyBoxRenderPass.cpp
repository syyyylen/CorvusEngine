#include "SkyBoxRenderPass.h"

#include "DDSTextureLoader/DDSTextureLoader.h"

void SkyBoxRenderPass::Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_textureSampler = renderer->CreateSampler(D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    
    GraphicsPipelineSpecs skyboxSpecs;
    skyboxSpecs.FormatCount = 1;
    skyboxSpecs.Formats[0] = TextureFormat::RGBA8;
    skyboxSpecs.BlendOperation = BlendOperation::None;
    skyboxSpecs.DepthEnabled = true;
    skyboxSpecs.Depth = DepthOperation::LEqual;
    skyboxSpecs.DepthFormat = TextureFormat::R32Depth;
    skyboxSpecs.Cull = CullMode::None;
    skyboxSpecs.Fill = FillMode::Solid;
    ShaderCompiler::CompileShader("Shaders/SkyBoxVertex.hlsl", ShaderType::Vertex, skyboxSpecs.ShadersBytecodes[ShaderType::Vertex]);
    ShaderCompiler::CompileShader("Shaders/SkyBoxPixel.hlsl", ShaderType::Pixel, skyboxSpecs.ShadersBytecodes[ShaderType::Pixel]);

    m_skyboxPipeline = renderer->CreateGraphicsPipeline(skyboxSpecs);

    m_constantBuffer = renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_constantBuffer);
    
    m_sphereMesh = std::make_shared<RenderItem>();
    m_sphereMesh->ImportMesh(renderer, "Assets/sphere.gltf");

    m_enviroMaps.SkyBox = renderer->LoadTextureCube(L"Assets/skymap.dds");

    m_enviroMaps.DiffuseIrradianceMap = renderer->CreateTextureCube(128, 128, TextureFormat::RGBA8);

    Shader cs;
    ShaderCompiler::CompileShader("Shaders/IrradianceComputeShader.hlsl", ShaderType::Compute, cs);
    auto csPipeline = renderer->CreateComputePipeline(cs);

    auto cmdList = renderer->CreateGraphicsCommandList();
    cmdList->Begin();
    cmdList->BindComputePipeline(csPipeline);
    cmdList->ImageBarrier(m_enviroMaps.DiffuseIrradianceMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->BindComputeUnorderedAccessView(m_enviroMaps.DiffuseIrradianceMap, 0, 0);
    cmdList->BindComputeShaderResource(m_enviroMaps.SkyBox, 1);
    cmdList->Dispatch(128 / 32, 128 / 32, 6);
    cmdList->ImageBarrier(m_enviroMaps.DiffuseIrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->End();

    renderer->ExecuteCommandBuffers({ cmdList }, D3D12_COMMAND_LIST_TYPE_DIRECT);
    renderer->WaitForGPU();
}

void SkyBoxRenderPass::Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget)
{
    auto view = camera.GetViewMatrix();
    auto proj = camera.GetProjMatrix();
    DirectX::XMMATRIX viewProj = view * proj;
    
    SkyBoxConstantBuffer constantBuffer;
    constantBuffer.CameraPosition = camera.GetPosition();
    DirectX::XMStoreFloat4x4(&constantBuffer.ViewProj, viewProj);

    void* data;
    m_constantBuffer->Map(0, 0, &data);
    memcpy(data, &constantBuffer, sizeof(SkyBoxConstantBuffer));
    m_constantBuffer->Unmap(0, 0);

    auto commandList = renderer->GetCurrentCommandList();

    commandList->SetViewport(0, 0, globalPassData.ViewportSizeX, globalPassData.ViewportSizeY);

    commandList->ImageBarrier(renderTarget.RenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->BindRenderTargets({ renderTarget.RenderTexture }, renderTarget.DepthBuffer);

    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_skyboxPipeline);
    commandList->BindConstantBuffer(m_constantBuffer, 0);
    commandList->BindGraphicsSampler(m_textureSampler, 2);
    commandList->BindGraphicsShaderResource(m_enviroMaps.SkyBox, 1);

    commandList->BindVertexBuffer(m_sphereMesh->GetPrimitives()[0].m_vertexBuffer);
    commandList->BindIndexBuffer(m_sphereMesh->GetPrimitives()[0].m_indicesBuffer);
    commandList->DrawIndexed(m_sphereMesh->GetPrimitives()[0].m_indexCount);
    
    commandList->ImageBarrier(renderTarget.RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void SkyBoxRenderPass::OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
}
