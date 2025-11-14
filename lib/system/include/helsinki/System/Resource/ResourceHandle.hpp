#pragma once

#include <string>

namespace hl
{
    class ResourceManager; // forward declaration

    template<typename T>
    class ResourceHandle
    {
    private:
        std::string resourceId;
        ResourceManager* resourceManager;

    public:
        ResourceHandle() : resourceManager(nullptr) {}

        ResourceHandle(const std::string& id, ResourceManager* manager)
            : resourceId(id), resourceManager(manager)
        {
        }

        T* Get() const;
        bool IsValid() const;
        const std::string& GetId() const;

        T* operator->() const { return Get(); }
        T& operator*() const { return *Get(); }
        operator bool() const { return IsValid(); }
    };
}