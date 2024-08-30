#pragma once
#include <map>

#include "Core.h"
#include "../RHI/D3D12Renderer.h"

class ResourcesManager
{
public:
    ResourcesManager(std::shared_ptr<D3D12Renderer> renderer);
    ~ResourcesManager();

    std::shared_ptr<Texture> LoadTexture(const std::string& texPath, Uploader& uploader, Image& img);
    
private:
    std::weak_ptr<D3D12Renderer> m_renderer;
    std::map<std::string, std::weak_ptr<Texture>> m_textures;
};
