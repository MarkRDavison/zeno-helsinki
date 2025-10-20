#pragma once

#include <string>

namespace hl
{
    class Resource
    {
    private:
        std::string resourceId;
        bool loaded = false;

    public:
        explicit Resource(const std::string& id) : resourceId(id) {}
        virtual ~Resource() = default;

        const std::string& GetId() const { return resourceId; }
        bool IsLoaded() const { return loaded; }

        virtual bool Load()
        {
            loaded = true;
            return true;
        }

        virtual void Unload()
        {
            loaded = false;
        }
    };
}