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

    // TODO remove all this
    auto cmdList = std::make_shared<CommandList>(renderer->GetDevice(), renderer->GetHeaps(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    cmdList->Begin();
    
    HRESULT hr = DirectX::CreateDDSTextureFromFile12(renderer->GetDevice()->GetDevice(), cmdList->GetCommandList(), L"Assets/skymap.dds",m_cubeMap.Resource, m_cubeMap.UploadHeap);
    if(FAILED(hr))
    {
        LOG(Error, "failed to create dds texture !!!");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    auto ShaderHeap = renderer->GetHeaps().ShaderHeap;
    m_cubeMap.Handle = ShaderHeap->Allocate();
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = m_cubeMap.Resource->GetDesc().MipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    srvDesc.Format = m_cubeMap.Resource->GetDesc().Format;
    renderer->GetDevice()->GetDevice()->CreateShaderResourceView(m_cubeMap.Resource.Get(), &srvDesc, m_cubeMap.Handle.CPU);

    cmdList->End();
    renderer->ExecuteCommandBuffers({ cmdList }, D3D12_COMMAND_LIST_TYPE_DIRECT);

    renderer->WaitForGPU();
    // TODO remove all this
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

    commandList->SetViewport(0, 0, globalPassData.viewportSizeX, globalPassData.viewportSizeY);

    commandList->ImageBarrier(renderTarget.RenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->BindRenderTargets({ renderTarget.RenderTexture }, renderTarget.DepthBuffer);

    commandList->SetTopology(Topology::TriangleList);
    commandList->BindGraphicsPipeline(m_skyboxPipeline);
    commandList->BindConstantBuffer(m_constantBuffer, 0);
    commandList->BindGraphicsSampler(m_textureSampler, 2);

    // TODO remove this, just for test
    commandList->GetCommandList()->SetGraphicsRootShaderResourceView(1, m_cubeMap.Handle.GPU.ptr);

    commandList->BindVertexBuffer(m_sphereMesh->GetPrimitives()[0].m_vertexBuffer);
    commandList->BindIndexBuffer(m_sphereMesh->GetPrimitives()[0].m_indicesBuffer);
    commandList->DrawIndexed(m_sphereMesh->GetPrimitives()[0].m_indexCount);
    
    commandList->ImageBarrier(renderTarget.RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void SkyBoxRenderPass::OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
}
