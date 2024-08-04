#pragma once
#include <Core.h>

#include "Buffer.h"
#include "Device.h"
#include "GraphicsPipeline.h"
#include "Texture.h"

enum class Topology
{
    LineList = D3D_PRIMITIVE_TOPOLOGY_LINELIST,
    LineStrip = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
    PointList = D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
    TriangleList = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    TriangleStrip = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
};

class CommandList
{
public:
    CommandList(std::shared_ptr<Device> device, const Heaps& heaps, D3D12_COMMAND_LIST_TYPE commandQueueType);
    ~CommandList();

    void Begin();
    void End();

    void ImageBarrier(std::shared_ptr<Texture> texture, D3D12_RESOURCE_STATES state);
    void BindRenderTargets(const std::vector<std::shared_ptr<Texture>> renderTargets, std::shared_ptr<Texture> depthTarget);
    void ClearRenderTarget(std::shared_ptr<Texture> renderTarget, float r, float g, float b, float a);
    void SetViewport(float x, float y, float width, float height);
    void SetTopology(Topology topology);
    void BindVertexBuffer(std::shared_ptr<Buffer> buffer);
    void BindIndexBuffer(std::shared_ptr<Buffer> buffer);
    void BindConstantBuffer(std::shared_ptr<Buffer> buffer, int idx);
    void BindGraphicsPipeline(std::shared_ptr<GraphicsPipeline> pipeline);
    void Draw(int vertexCount);
    void DrawIndexed(int indexCount);
    void CopyTextureToTexture(std::shared_ptr<Texture> dst, std::shared_ptr<Texture> src);
    void CopyBufferToBuffer(std::shared_ptr<Buffer> dst, std::shared_ptr<Buffer> src);

    ID3D12GraphicsCommandList* GetCommandList() { return m_commandList; }

private:
    ID3D12CommandAllocator* m_commandAllocator;
    ID3D12GraphicsCommandList* m_commandList;
    D3D12_COMMAND_LIST_TYPE m_type;
    Heaps m_heaps;
};