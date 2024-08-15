#include "CommandList.h"

CommandList::CommandList(std::shared_ptr<Device> device, const Heaps& heaps, D3D12_COMMAND_LIST_TYPE commandQueueType) : m_type(commandQueueType), m_heaps(heaps)
{
    HRESULT hr = device->GetDevice()->CreateCommandAllocator(m_type, IID_PPV_ARGS(&m_commandAllocator));
    if(FAILED(hr))
    {
        LOG(Error, "CommandBuffer : failed to create command allocator !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    hr = device->GetDevice()->CreateCommandList(0, m_type, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList));
    if(FAILED(hr))
    {
        LOG(Error, "CommandBuffer : failed to create command list !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }

    hr = m_commandList->Close();
    if(FAILED(hr))
    {
        LOG(Error, "CommandBuffer : failed to close command list !");
        std::string errorMsg = std::system_category().message(hr);
        LOG(Error, errorMsg);
    }
}

CommandList::~CommandList()
{
    m_commandList->Release();
    if(m_commandAllocator)
        m_commandAllocator->Release();
}

void CommandList::Begin()
{
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator, nullptr);
    if (m_type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
        ID3D12DescriptorHeap* heaps[] = {
            m_heaps.ShaderHeap->GetHeap() /*,
            m_heaps.SamplerHeap->GetHeap()*/
        };
        m_commandList->SetDescriptorHeaps(1, heaps); // TODO add sampler heap
    }
}

void CommandList::End()
{
    m_commandList->Close();
}

void CommandList::ImageBarrier(std::shared_ptr<Texture> texture, D3D12_RESOURCE_STATES state)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = texture->GetResource().Resource;
    barrier.Transition.StateBefore = texture->GetState();
    barrier.Transition.StateAfter = state;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    if (barrier.Transition.StateBefore == barrier.Transition.StateAfter)
        return;

    m_commandList->ResourceBarrier(1, &barrier);

    texture->SetState(state);
}

void CommandList::BindRenderTargets(const std::vector<std::shared_ptr<Texture>>& renderTargets, std::shared_ptr<Texture> depthTarget)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvDescriptors;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor;

    for (auto& renderTarget : renderTargets) {
        rtvDescriptors.push_back(renderTarget->m_rtv.CPU);
    }
    if (depthTarget) {
        dsvDescriptor = depthTarget->m_dsv.CPU;
    }

    m_commandList->OMSetRenderTargets(rtvDescriptors.size(), rtvDescriptors.data(), false, depthTarget ? &dsvDescriptor : nullptr);
}

void CommandList::ClearRenderTarget(std::shared_ptr<Texture> renderTarget, float r, float g, float b, float a)
{
    float clearValues[4] = { r, g, b, a };
    m_commandList->ClearRenderTargetView(renderTarget->m_rtv.CPU, clearValues, 0, nullptr);
}

void CommandList::ClearDepthTarget(std::shared_ptr<Texture> depthTarget)
{
    m_commandList->ClearDepthStencilView(depthTarget->m_dsv.CPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void CommandList::SetViewport(float x, float y, float width, float height)
{
    D3D12_VIEWPORT Viewport = {};
    Viewport.Width = width;
    Viewport.Height = height;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    Viewport.TopLeftX = x;
    Viewport.TopLeftY = y;

    D3D12_RECT Rect;
    Rect.right = width;
    Rect.bottom = height;
    Rect.top = 0.0f;
    Rect.left = 0.0f;

    m_commandList->RSSetViewports(1, &Viewport);
    m_commandList->RSSetScissorRects(1, &Rect);
}

void CommandList::SetTopology(Topology topology)
{
    m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY(topology));
}

void CommandList::BindVertexBuffer(std::shared_ptr<Buffer> buffer)
{
    m_commandList->IASetVertexBuffers(0, 1, &buffer->m_VBV);
}

void CommandList::BindIndexBuffer(std::shared_ptr<Buffer> buffer)
{
    m_commandList->IASetIndexBuffer(&buffer->m_IBV);
}

void CommandList::BindConstantBuffer(std::shared_ptr<Buffer> buffer, int idx)
{
    m_commandList->SetGraphicsRootDescriptorTable(idx, buffer->m_descriptorHandle.GPU);
}

void CommandList::BindGraphicsPipeline(std::shared_ptr<GraphicsPipeline> pipeline)
{
    m_commandList->SetPipelineState(pipeline->GetPipelineState());
    m_commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
}

void CommandList::Draw(int vertexCount)
{
    m_commandList->DrawInstanced(vertexCount, 1, 0, 0);
}

void CommandList::DrawIndexed(int indexCount)
{
    m_commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void CommandList::CopyTextureToTexture(std::shared_ptr<Texture> dst, std::shared_ptr<Texture> src)
{
    D3D12_TEXTURE_COPY_LOCATION BlitSource = {};
    BlitSource.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    BlitSource.pResource = src->GetResource().Resource;
    BlitSource.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION BlitDest = {};
    BlitDest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    BlitDest.pResource = dst->GetResource().Resource;
    BlitDest.SubresourceIndex = 0;

    m_commandList->CopyTextureRegion(&BlitDest, 0, 0, 0, &BlitSource, nullptr);
}

void CommandList::CopyBufferToBuffer(std::shared_ptr<Buffer> dst, std::shared_ptr<Buffer> src)
{
    m_commandList->CopyResource(dst->GetResource().Resource, src->GetResource().Resource);
}