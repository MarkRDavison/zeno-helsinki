#include <Systems/PlayerControlSystem.hpp>
#include <Components/EntityComponent.hpp>
#include <HurricaneConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

namespace hur
{

	PlayerControlSystem::PlayerControlSystem(
		hl::InputManager& inputManager,
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_inputManager(inputManager),
		_eventBus(eventBus),
		_scene(scene)
	{

	}

	void PlayerControlSystem::update(float delta)
	{
		auto b = _scene.getEntity("Player");

		if (b == nullptr)
		{
			return;
		}

		auto tc = b->GetComponent<hl::TransformComponent>();
		auto sc = b->GetComponent<EntityComponent>();

		glm::vec2 movement{};

		const float SPEED = 512.0f;

		if (_inputManager.isKeyDown(GLFW_KEY_A))
		{
			movement.x -= 1.0f;
		}

		if (_inputManager.isKeyDown(GLFW_KEY_D))
		{
			movement.x += 1.0f;
		}
		if (_inputManager.isKeyDown(GLFW_KEY_W))
		{
			movement.y -= 1.0f;
		}

		if (_inputManager.isKeyDown(GLFW_KEY_S))
		{
			movement.y += 1.0f;
		}

		if (movement.x != 0.0f || movement.y != 0.0f)
		{

			const auto pos = tc->GetPosition();

			movement = glm::normalize(movement) * delta * SPEED;

			auto newPosition = glm::vec2(pos.x + movement.x, pos.y + movement.y);

			if (newPosition.x - sc->Size.x / 2.0f < 0.0f)
			{
				newPosition.x = sc->Size.x / 2.0f;
			}
			else if (newPosition.x + sc->Size.x / 2.0f > HurricaneConstants::Width)
			{
				newPosition.x = HurricaneConstants::Width - sc->Size.x / 2.0f;
			}

			if (newPosition.y - sc->Size.y / 2.0f < 0.0f)
			{
				newPosition.y = sc->Size.y / 2.0f;
			}
			else if (newPosition.y + sc->Size.y / 2.0f > HurricaneConstants::Height)
			{
				newPosition.y = HurricaneConstants::Height - sc->Size.y / 2.0f;
			}

			tc->SetPosition({ newPosition, pos.z });
		}
	}

}