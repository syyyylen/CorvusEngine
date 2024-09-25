#pragma once
#include <Core.h>

#include "Device.h"
#include "../Rendering/ShaderCompiler.h"

class ComputePipeline
{
public:
    ComputePipeline(std::shared_ptr<Device> device, Shader& shader);
    ~ComputePipeline();

    ID3D12PipelineState* GetPipelineState() { return m_pipelineState; }
    ID3D12RootSignature* GetRootSignature() { return m_rootSignature; }

private:
    ID3D12PipelineState* m_pipelineState;
    ID3D12RootSignature* m_rootSignature;
};
