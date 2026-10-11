#pragma once

#include <helsinki/System/Export.hpp>

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

    class EventListener
    {
    public:
        virtual ~EventListener() = default;
        virtual void OnEvent(const Event& event) = 0;
    };

}