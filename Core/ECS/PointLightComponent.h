#pragma once
#include <Core.h>
#include "Component.h"
#include "../../Rendering/RenderingLayouts.h"

class PointLightComponent : public Component
{
public:
    PointLightComponent();
    ~PointLightComponent() override;

    PointLight m_pointLight;
};
