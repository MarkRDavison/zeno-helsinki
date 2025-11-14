#pragma once

#include <helsinki/System/Resource/Resource.hpp>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <string>

namespace hl
{
    template<typename T>
    class ResourceHandle;

    class ResourceManager
    {
    public:
        template<typename T, typename... Args>
        ResourceHandle<T> Load(const std::string& resourceId, Args&&... args)
        {
            static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

            auto& typeResources = resources[std::type_index(typeid(T))];
            auto it = typeResources.find(resourceId);

            if (it != typeResources.end())
            {
                refCounts[resourceId]++;
                return ResourceHandle<T>(resourceId, this);
            }

            auto resource = std::make_shared<T>(resourceId, std::forward<Args>(args)...);

            if (!resource->Load())
            {
                return ResourceHandle<T>();
            }

            typeResources[resourceId] = resource;
            refCounts[resourceId] = 1;

            return ResourceHandle<T>(resourceId, this);
        }
        template<typename T, typename TBase, typename... Args>
        ResourceHandle<TBase> LoadAs(const std::string& resourceId, Args&&... args)
        {
            static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");
            static_assert(std::is_base_of<Resource, TBase>::value, "TBase must derive from Resource");
            static_assert(std::is_base_of<TBase, T>::value, "TBase must derive from T");

            auto& typeResources = resources[std::type_index(typeid(TBase))];
            auto it = typeResources.find(resourceId);

            if (it != typeResources.end())
            {
                refCounts[resourceId]++;
                return ResourceHandle<TBase>(resourceId, this);
            }

            auto resource = std::make_shared<T>(resourceId, std::forward<Args>(args)...);

            if (!resource->Load())
            {
                return ResourceHandle<TBase>();
            }

            typeResources[resourceId] = resource;
            refCounts[resourceId] = 1;

            return ResourceHandle<TBase>(resourceId, this);
        }

        template<typename T>
        T* GetResource(const std::string& resourceId)
        {
            auto& typeResources = resources[std::type_index(typeid(T))];
            auto it = typeResources.find(resourceId);

            if (it != typeResources.end())
            {
                return static_cast<T*>(it->second.get());
            }

            return nullptr;
        }

        template<typename T, typename TBase>
        T* GetResourceAs(const std::string& resourceId)
        {
            return reinterpret_cast<T*>(GetResource<TBase>(resourceId));
        }

        template<typename T>
        bool HasResource(const std::string& resourceId)
        {
            auto& typeResources = resources[std::type_index(typeid(T))];
            return typeResources.find(resourceId) != typeResources.end();
        }

        void Release(const std::string& resourceId)
        {
            auto it = refCounts.find(resourceId);
            if (it != refCounts.end())
            {
                it->second--;

                if (it->second <= 0)
                {
                    for (auto& [type, typeResources] : resources)
                    {
                        auto resourceIt = typeResources.find(resourceId);
                        if (resourceIt != typeResources.end())
                        {
                            resourceIt->second->Unload();
                            typeResources.erase(resourceIt);
                            break;
                        }
                    }

                    refCounts.erase(it);
                }
            }
        }

        void UnloadAll()
        {
            for (auto& [type, typeResources] : resources)
            {
                for (auto& [id, resource] : typeResources)
                {
                    resource->Unload();
                }
                typeResources.clear();
            }
            refCounts.clear();
        }

    private:
        std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;
        std::unordered_map<std::string, int> refCounts;
    };
}
