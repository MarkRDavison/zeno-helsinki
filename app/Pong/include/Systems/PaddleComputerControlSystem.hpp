#pragma once

#include <helsinki/Engine/ECS/System.hpp>

#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace pong
{
	class PaddleComputerControlSystem : public hl::System
	{
	public:
		PaddleComputerControlSystem(hl::InputManager& inputManager, hl::Scene& scene);
		void update(float delta) override;

	private:
		hl::InputManager& _inputManager;
		hl::Scene& _scene;
	};
}