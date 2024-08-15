#include "RenderItem.h"

RenderItem::RenderItem()
{
}

RenderItem::~RenderItem()
{
}

void RenderItem::ImportMesh(std::shared_ptr<D3D12Renderer> renderer, std::string filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG(Error, "Failed assimp import");
        return;
    }
    
    ProcessNode(renderer, scene->mRootNode, scene);
}

void RenderItem::ProcessPrimitive(std::shared_ptr<D3D12Renderer> renderer, aiMesh* mesh, const aiScene* scene)
{
    Primitive out;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.Position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if (mesh->HasNormals())
        {
            vertex.Normals = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            // TODO tangent and binormal
        }
        
        if (mesh->mTextureCoords[0])
            vertex.UV = DirectX::XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        
        vertices.push_back(vertex);
    }

    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    out.m_vertexCount = vertices.size();
    out.m_indexCount = indices.size();

    out.m_vertexBuffer = renderer->CreateBuffer(out.m_vertexCount * sizeof(Vertex), sizeof(Vertex), BufferType::Vertex, false);
    out.m_indicesBuffer = renderer->CreateBuffer(out.m_indexCount * sizeof(uint32_t), 0, BufferType::Index, false);

    Uploader uploader = renderer->CreateUploader();
    uploader.CopyHostToDeviceLocal(vertices.data(), vertices.size() * sizeof(Vertex), out.m_vertexBuffer);
    uploader.CopyHostToDeviceLocal(indices.data(), indices.size() * sizeof(uint32_t), out.m_indicesBuffer);
    renderer->FlushUploader(uploader);

    m_primitives.push_back(out);
}

void RenderItem::ProcessNode(std::shared_ptr<D3D12Renderer> renderer, aiNode* node, const aiScene* scene)
{
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; // TODO compute transform
        ProcessPrimitive(renderer, mesh, scene);
    }
}