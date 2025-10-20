#pragma once

#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/System/Resource/Resource.hpp>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace hl
{
    class ResourceManager
    {
    public:
        template<typename T>
        ResourceHandle<T> Load(const std::string& resourceId)
        {
            static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

            // Step 3a: Check existing resource cache to avoid redundant loading
            auto& typeResources = resources[std::type_index(typeid(T))];
            auto it = typeResources.find(resourceId);

            if (it != typeResources.end())
            {
                // Resource exists in cache - increment reference count and return handle
                refCounts[resourceId]++;
                return ResourceHandle<T>(resourceId, this);
            }

            // Step 3b: Create new resource instance and attempt loading
            auto resource = std::make_shared<T>(resourceId);
            if (!resource->Load())
            {
                // Loading failed - return invalid handle rather than corrupting cache
                return ResourceHandle<T>();
            }

            // Step 3c: Cache successful resource and initialize reference tracking
            typeResources[resourceId] = resource;
            refCounts[resourceId] = 1;

            return ResourceHandle<T>(resourceId, this);
        }

        template<typename T>
        T* GetResource(const std::string& resourceId)
        {
            // Access type-specific resource container using compile-time type information
            auto& typeResources = resources[std::type_index(typeid(T))];
            auto it = typeResources.find(resourceId);

            if (it != typeResources.end())
            {
                // Resource found - perform safe downcast and return typed pointer
                return static_cast<T*>(it->second.get());
            }

            // Resource not found - return null for safe handling by caller
            return nullptr;
        }

        template<typename T>
        bool HasResource(const std::string& resourceId)
        {
            // Efficient existence check without resource access overhead
            auto& typeResources = resources[std::type_index(typeid(T))];
            return typeResources.find(resourceId) != typeResources.end();
        }

        void Release(const std::string& resourceId)
        {
            // Locate reference count entry for this resource
            auto it = refCounts.find(resourceId);
            if (it != refCounts.end())
            {
                it->second--;

                // Check if resource has no remaining references
                if (it->second <= 0)
                {
                    // Step 5a: Locate and unload the unreferenced resource across all type containers
                    for (auto& [type, typeResources] : resources)
                    {
                        auto resourceIt = typeResources.find(resourceId);
                        if (resourceIt != typeResources.end())
                        {
                            resourceIt->second->Unload();      // Allow resource to clean up its data
                            typeResources.erase(resourceIt);   // Remove from cache
                            break;
                        }
                    }

                    // Step 5b: Clean up reference counting entry
                    refCounts.erase(it);
                }
            }
        }

        void UnloadAll()
        {
            // Emergency cleanup method for system shutdown or major state changes
            for (auto& [type, typeResources] : resources)
            {
                for (auto& [id, resource] : typeResources)
                {
                    resource->Unload();     // Ensure all resources clean up properly
                }
                typeResources.clear();      // Clear type-specific containers
            }
            refCounts.clear();              // Reset all reference counts
        }

    private:
        // Two-level storage system: organize by type first, then by unique identifier
        // This approach enables type-safe resource access while maintaining efficient lookup
        std::unordered_map<std::type_index,
            std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;

        // Reference counting system for automatic resource lifecycle management
        // Maps resource IDs to their current usage count for garbage collection
        std::unordered_map<std::string, int> refCounts;
    };
}