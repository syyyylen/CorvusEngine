#include "SSAORenderPass.h"

void SSAORenderPass::Initialize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    Shader SSAOShader;
    ShaderCompiler::CompileShader("Shaders/SSAO.hlsl", ShaderType::Compute, SSAOShader);
    m_SSAOPipeline = renderer->CreateComputePipeline(SSAOShader);

    m_constantBuffer = renderer->CreateBuffer(256, 0, BufferType::Constant, false);
    renderer->CreateConstantBuffer(m_constantBuffer);

    OnResize(renderer, width, height);
}

void SSAORenderPass::OnResize(std::shared_ptr<D3D12Renderer> renderer, int width, int height)
{
    m_SSAOTexture.reset();
    
    m_SSAOTexture = renderer->CreateTexture(width, height, TextureFormat::R16Norm, TextureType::Storage);
    renderer->CreateUnorderedAccessView(m_SSAOTexture);
    renderer->CreateShaderResourceView(m_SSAOTexture);
}

void SSAORenderPass::Pass(std::shared_ptr<D3D12Renderer> renderer, const GlobalPassData& globalPassData, const Camera& camera, const std::vector<RenderMeshData>& renderMeshesData, RenderTargetInfo renderTarget)
{
    SSAOConstantBuffer constantBuffer;
    constantBuffer.Value = 0.7f;

    void* data;
    m_constantBuffer->Map(0, 0, &data);
    memcpy(data, &constantBuffer, sizeof(SSAOConstantBuffer));
    m_constantBuffer->Unmap(0, 0);

    auto commandList = renderer->GetCurrentCommandList();

    commandList->BindComputePipeline(m_SSAOPipeline);
    commandList->ImageBarrier(m_SSAOTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->BindComputeUnorderedAccessView(m_SSAOTexture, 0);
    commandList->BindComputeConstantBuffer(m_constantBuffer, 1);
    
    commandList->Dispatch(m_width / 32, m_height / 32, 6);
    
    commandList->ImageBarrier(m_SSAOTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
}