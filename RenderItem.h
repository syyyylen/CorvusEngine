#pragma once
#include "Core.h"
#include "RHI/D3D12Renderer.h"

struct Primitive
{
    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indicesBuffer;
    int m_vertexCount;
    int m_indexCount;
};

class RenderItem
{
public:
    RenderItem();
    ~RenderItem();

    std::vector<Primitive> m_primitives;
};
