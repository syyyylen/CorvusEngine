#pragma once
#include <Core.h>
#include "../Rendering/RenderItem.h"
#include "Component.h"

class MeshComponent : public Component
{
public:
    MeshComponent();
    ~MeshComponent();

    void SetRenderItem(std::shared_ptr<RenderItem> renderItem) { m_renderItem = renderItem; }
    std::shared_ptr<RenderItem> GetRenderItem() { return m_renderItem; }
    
private:
    std::shared_ptr<RenderItem> m_renderItem;
};
