#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <string>

namespace hl
{
    class Entity
    {
    public:
        void setName(const std::string& name) { _name = name; }
        const std::string& getName() const { return _name; }

        void AddTag(const std::string& tag);
        void ClearTag(const std::string& tag);
        bool HasTag(const std::string& tag) const;

        template<typename T, typename... Args>
        T* AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

            size_t typeID = Component::GetTypeID<T>();

            // Check if component of this type already exists
            auto it = _componentMap.find(typeID);
            if (it != _componentMap.end())
            {
                return static_cast<T*>(it->second);
            }

            // Create new component
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T* componentPtr = component.get();
            _componentMap[typeID] = componentPtr;
            _components.push_back(std::move(component));
            return componentPtr;
        }

        template<typename T>
        T* GetComponent()
        {
            size_t typeID = Component::GetTypeID<T>();
            auto it = _componentMap.find(typeID);
            if (it != _componentMap.end())
            {
                return static_cast<T*>(it->second);
            }
            return nullptr;
        }


        template<typename... Args>
        bool HasComponents() const
        {
            return (HasComponent<Args>() && ...);
        }

        template<typename T>
        bool HasComponent() const
        {
            const auto c = GetComponent<T>();
            return c != nullptr;
        }

        template<typename T>
        bool RemoveComponent()
        {
            size_t typeID = Component::GetTypeID<T>();
            auto it = _componentMap.find(typeID);
            if (it != _componentMap.end())
            {
                Component* componentPtr = it->second;
                _componentMap.erase(it);

                for (auto compIt = _components.begin(); compIt != _components.end(); ++compIt)
                {
                    if (compIt->get() == componentPtr)
                    {
                        _components.erase(compIt);
                        return true;
                    }
                }
            }
            return false;
        }

    private:
        std::string _name;
        std::vector<std::unique_ptr<Component>> _components;
        std::unordered_map<size_t, Component*> _componentMap;
        std::unordered_set<uint32_t> tags;
    };
}