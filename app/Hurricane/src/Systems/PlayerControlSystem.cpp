#include <Systems/PlayerControlSystem.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <GLFW/glfw3.h>

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

			tc->SetPosition({ pos.x + movement.x, pos.y + movement.y, pos.z });
		}
	}

}