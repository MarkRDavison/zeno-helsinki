#pragma once

namespace hl
{
    class EventListener
    {
    public:
        virtual ~EventListener() = default;
        virtual void OnEvent(const Event& event) = 0;
    };
}