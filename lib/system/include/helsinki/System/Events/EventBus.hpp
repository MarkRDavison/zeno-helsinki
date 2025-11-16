#pragma once

#include <helsinki/System/Events/Event.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

namespace hl
{

    class EventBus
    {
    private:
        std::vector<EventListener*> listeners;
        std::queue<std::unique_ptr<Event>> eventQueue;
        std::mutex queueMutex;
        bool immediateMode = true;

    public:
        void SetImmediateMode(bool immediate)
        {
            immediateMode = immediate;
        }

        void AddListener(EventListener* listener)
        {
            listeners.push_back(listener);
        }

        void RemoveListener(EventListener* listener)
        {
            auto it = std::find(listeners.begin(), listeners.end(), listener);
            if (it != listeners.end())
            {
                listeners.erase(it);
            }
        }

        void PublishEvent(const Event& event)
        {
            if (immediateMode)
            {
                for (auto listener : listeners)
                {
                    listener->OnEvent(event);
                }
            }
            else
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                eventQueue.push(std::unique_ptr<Event>(event.Clone()));
            }
        }

        void ProcessEvents()
        {
            if (immediateMode)
            {
                return;
            }

            std::queue<std::unique_ptr<Event>> currentEvents;

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                std::swap(currentEvents, eventQueue);
            }

            while (!currentEvents.empty())
            {
                auto& event = *currentEvents.front();

                for (auto listener : listeners)
                {
                    listener->OnEvent(event);
                }

                currentEvents.pop();
            }
        }
    };

}