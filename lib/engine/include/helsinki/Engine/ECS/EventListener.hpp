#pragma once

#include <helsinki/Engine/Export.hpp>

namespace hl
{
    class EventListener
    {
    public:
        virtual ~EventListener() = default;
        virtual void OnEvent(const Event& event) = 0;
    };
}