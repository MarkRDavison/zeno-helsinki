#pragma once

#include <helsinki/Engine/ECS/ComponentTypeIDSystem.hpp>

namespace hl
{
    class Entity;
    class Component
    {
    public:
        enum class State
        {
            Uninitialized,
            Initializing,
            Active,
            Destroying,
            Destroyed
        };

    private:
        State state = State::Uninitialized;
        Entity* owner = nullptr;

    public:
        virtual ~Component()
        {
            if (state != State::Destroyed)
            {
                OnDestroy();
                state = State::Destroyed;
            }
        }

        void Initialize()
        {
            if (state == State::Uninitialized)
            {
                state = State::Initializing;
                OnInitialize();
                state = State::Active;
            }
        }

        void Destroy()
        {
            if (state == State::Active)
            {
                state = State::Destroying;
                OnDestroy();
                state = State::Destroyed;
            }
        }

        bool IsActive() const { return state == State::Active; }

        void SetOwner(Entity* entity) { owner = entity; }
        Entity* GetOwner() const { return owner; }

        template<typename T>
        static size_t GetTypeID()
        {
            return ComponentTypeIDSystem::GetTypeID<T>();
        }

    protected:
        virtual void OnInitialize() {}
        virtual void OnDestroy() {}
        virtual void Update(float) {}
        virtual void Render() {}

        friend class Entity; // Allow Entity to call protected methods
    };
}