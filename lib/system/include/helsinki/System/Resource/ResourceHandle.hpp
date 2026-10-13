#pragma once

#include <string>

namespace hl
{
    class ResourceManager;

    template<typename T>
    class ResourceHandle
    {
    private:
        std::string resourceId;
        ResourceManager* resourceManager;

    public:
        ResourceHandle();
        ResourceHandle(const std::string& id, ResourceManager* manager);

        T* Get() const;
        bool IsValid() const;

        const std::string& GetId() const
        {
            return resourceId;
        }

        T* operator->() const;
        T& operator*() const;
        operator bool() const;
    };
}
