#include "GameObject.h"

GameObject::GameObject(std::string name) : m_name(name)
{
}

GameObject::~GameObject()
{
    m_components.clear();
}
