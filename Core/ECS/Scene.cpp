#include "Scene.h"

Scene::Scene(std::string name) : m_name(name)
{
}

Scene::~Scene()
{
    m_gameObjects.clear();
}

std::shared_ptr<GameObject> Scene::CreateGameObject(std::string name, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale)
{
    name += std::to_string(m_gameObjects.size());
    std::shared_ptr<GameObject> go = std::make_shared<GameObject>(name);
    m_gameObjects.emplace_back(go);

    auto tfComp = go->AddComponent<TransformComponent>();
    DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
    mat *= DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(rotation.x), DirectX::XMConvertToRadians(rotation.y),DirectX::XMConvertToRadians(rotation.z));
    mat *= DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    mat *= DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    DirectX::XMStoreFloat4x4(&tfComp->m_transform, mat);
    
    return go;
}

void Scene::RemoveGameObject(std::shared_ptr<GameObject> goToRemove)
{
    m_gameObjects.erase(std::remove(m_gameObjects.begin(), m_gameObjects.end(), goToRemove), m_gameObjects.end());
}
