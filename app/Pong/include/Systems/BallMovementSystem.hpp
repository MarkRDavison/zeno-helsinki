#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/System/Events/EventBus.hpp>

namespace pong
{
	class BallMovementSystem : public hl::System
	{
	public:
		BallMovementSystem(
			hl::InputManager& inputManager, 
			hl::EventBus& eventBus,
			hl::Scene& scene);
		void update(float delta) override;

	private:
		hl::InputManager& _inputManager;
		hl::EventBus& _eventBus;
		hl::Scene& _scene;
	};
}