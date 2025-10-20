#pragma once

namespace hl
{
    class ComponentTypeIDSystem
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