#pragma once
#include "Core.h"
#include "RHI/D3D12Renderer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 UV;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Tangent;
    DirectX::XMFLOAT3 Binormal;
};

struct Primitive
{
    DirectX::XMFLOAT4X4 Transform;
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

    void ImportMesh(std::shared_ptr<D3D12Renderer> renderer, std::string filePath);

private:
    void ProcessPrimitive(std::shared_ptr<D3D12Renderer> renderer, aiMesh *mesh, const aiScene *scene);
    void ProcessNode(std::shared_ptr<D3D12Renderer> renderer, aiNode *node, const aiScene *scene);
};
