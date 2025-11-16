#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

    class EventDispatcher
    {
    private:
        const Event& event;

    public:
        explicit EventDispatcher(const Event& e) : event(e) {}

        template<typename T, typename F>
        bool Dispatch(const F& handler)
        {
            if (event.GetType() == T::GetStaticType())
            {
                handler(static_cast<const T&>(event));
                return true;
            }

            return false;
        }
    };

}