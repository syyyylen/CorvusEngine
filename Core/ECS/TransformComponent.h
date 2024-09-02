#pragma once
#include "Component.h"

class TransformComponent : public Component
{
public:
    TransformComponent();
    ~TransformComponent();

    DirectX::XMFLOAT4X4 m_transform;
};
