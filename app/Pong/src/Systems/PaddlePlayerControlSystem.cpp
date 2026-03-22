#include <Systems/PaddlePlayerControlSystem.hpp>
#include <GLFW/glfw3.h>
#include <PongConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <Components/EntityComponent.hpp>

namespace pong
{
	PaddlePlayerControlSystem::PaddlePlayerControlSystem(
		hl::InputManager& inputManager,
		hl::Scene& scene
	) :
		_inputManager(inputManager),
		_scene(scene)
	{

	}

	void PaddlePlayerControlSystem::update(float delta)
	{
		float posOffset = 0.0f;

		if (_inputManager.isKeyDown(GLFW_KEY_W))
		{
			posOffset += 1.0f;
		}
		if (_inputManager.isKeyDown(GLFW_KEY_S))
		{
			posOffset -= 1.0f;
		}

		if (posOffset != 0.0f)
		{
			auto offset = PongConstants::PaddleMoveSpeedBase * posOffset * delta;

			for (auto& e : _scene.getEntities())
			{
				if (e->HasComponents<hl::TransformComponent, EntityComponent>())
				{
					auto tc = e->GetComponent<hl::TransformComponent>();
					auto ec = e->GetComponent<EntityComponent>();

					if (ec->ControlledState == ControlledState::PLAYER)
					{
						const auto& pos = tc->GetPosition();

						auto newPos = pos - glm::vec3(0.0f, offset, 0.0f);

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
}