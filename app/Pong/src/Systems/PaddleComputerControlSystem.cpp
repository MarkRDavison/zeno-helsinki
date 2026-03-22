#include <Systems/PaddleComputerControlSystem.hpp>
#include <PongConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <Components/EntityComponent.hpp>

namespace pong
{
	PaddleComputerControlSystem::PaddleComputerControlSystem(
		hl::InputManager& inputManager,
		hl::Scene& scene
	) :
		_inputManager(inputManager),
		_scene(scene)
	{

	}

	void PaddleComputerControlSystem::update(float delta)
	{
		float posOffset = 0.0f;

		auto b = _scene.getEntity("Ball"); // TODO: Handle multiple balls
		auto btc = b->GetComponent<hl::TransformComponent>();

		auto ballPositionY = btc->GetPosition().y;

		auto offset = PongConstants::PaddleMoveSpeedBase * posOffset * delta;

		for (auto& e : _scene.getEntities())
		{
			if (e->HasComponents<hl::TransformComponent, EntityComponent>())
			{
				auto tc = e->GetComponent<hl::TransformComponent>();
				auto ec = e->GetComponent<EntityComponent>();

				if (ec->ControlledState == ControlledState::COMPUTER)
				{
					const auto& pos = tc->GetPosition();

					auto newPos = glm::vec3(pos.x, ballPositionY, 0.0f);

					if (newPos.y < 0.0f)
					{
						newPos.y = 0.0f;
					}

					if (newPos.y + PongConstants::PaddleHeight > PongConstants::GameBoundsHeight)
					{
						newPos.y = PongConstants::GameBoundsHeight - PongConstants::PaddleHeight;
					}

					tc->SetPosition(newPos);
				}
			}
		}
	}
}