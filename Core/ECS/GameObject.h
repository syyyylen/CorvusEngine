#pragma once
#include <Core.h>

#include "Component.h"

class GameObject
{
public:
    GameObject(std::string name);
    ~GameObject();

    template<typename T>
    std::shared_ptr<T> AddComponent()
    {
        std::shared_ptr<T> newComp = std::make_shared<T>();
        m_components.emplace_back(newComp);
        return newComp;
    }

    template<typename T>
    std::shared_ptr<T> GetComponent()
    {
        for(auto component : m_components)
        {
            if(auto typedComponent = std::dynamic_pointer_cast<T>(component))
                return typedComponent;
        }

        return nullptr;
    }

private:
    std::string m_name;
    std::vector<std::shared_ptr<Component>> m_components;
};
