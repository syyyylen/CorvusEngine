#pragma once
#include <Core.h>

#include "GameObject.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "Component.h"

class Scene
{
public:
    Scene(std::string name);
    ~Scene();

    std::shared_ptr<GameObject> CreateGameObject(std::string name,
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f },
        DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f },
        DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f });
    void RemoveGameObject(std::shared_ptr<GameObject> goToRemove);

    std::vector<std::shared_ptr<GameObject>> m_gameObjects;

private:
    std::string m_name;
};
