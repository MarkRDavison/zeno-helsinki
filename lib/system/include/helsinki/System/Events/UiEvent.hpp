#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

    class UiEvent : public Event
    {
    private:
        void* target;
        bool bubbles;
        bool cancelBubble = false;

    public:
        UiEvent(void* targetElement, bool bubbling = true)
            : target(targetElement), bubbles(bubbling)
        {
        }

        void* GetTarget() const { return target; }
        bool Bubbles() const { return bubbles; }

        void StopPropagation()
        {
            cancelBubble = true;
        }

        bool IsPropagationStopped() const
        {
            return cancelBubble;
        }

        DEFINE_EVENT_TYPE(UIEvent)
    };

}