#pragma once

namespace hl
{
    template<typename T>
    T* ResourceHandle<T>::Get() const
    {
        if (!resourceManager) return nullptr;
        return resourceManager->GetResource<T>(resourceId);
    }

    template<typename T>
    bool ResourceHandle<T>::IsValid() const
    {
        return resourceManager && resourceManager->HasResource<T>(resourceId);
    }

    template<typename T>
    const std::string& ResourceHandle<T>::GetId() const
    {
        return resourceId;
    }
}
