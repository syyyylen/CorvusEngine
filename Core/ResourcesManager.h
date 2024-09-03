#pragma once
#include <map>

#include "Core.h"
#include "../Rendering/RenderItem.h"
#include "../RHI/D3D12Renderer.h"

class ResourcesManager
{
public:
    ResourcesManager(std::shared_ptr<D3D12Renderer> renderer);
    ~ResourcesManager();

    std::shared_ptr<Texture> LoadTexture(const std::string& texPath, Uploader& uploader, Image& img);
    std::shared_ptr<RenderItem> LoadMesh(const std::string& meshPath);
    
private:
    std::weak_ptr<D3D12Renderer> m_renderer;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::weak_ptr<RenderItem>> m_renderItems;
};
