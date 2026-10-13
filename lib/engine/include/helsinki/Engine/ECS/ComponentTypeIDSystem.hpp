#pragma once

#include <helsinki/Engine/Export.hpp>

#include <cstddef>

namespace hl
{
    class HELSINKI_ENGINE_EXPORT ComponentTypeIDSystem
    {
    private:
        static size_t nextTypeID;

    public:
        template<typename T>
        static size_t GetTypeID()
        {
            static size_t typeID = nextTypeID++;
            return typeID;
        }
    };
}