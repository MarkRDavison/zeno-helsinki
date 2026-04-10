#include <Systems/PaddleComputerControlSystem.hpp>
#include <PongConstants.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <iostream>

namespace pong
{
	glm::vec3 handleFollowBrain(glm::vec3 pos, glm::vec3 ballPosition, float delta);
	glm::vec3 handleShortSightedCenterBrain(glm::vec3 pos, glm::vec3 ballPosition, float delta);
	glm::vec3 handleLazyBrain(glm::vec3 pos, glm::vec3 ballPosition, glm::vec3 ballVelocity, float delta);
	
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

		auto ballPosition = btc->GetPosition();
		auto ballVelocity = b->GetComponent<hl::KinematicComponent>()->velocity;

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

					auto newPos = glm::vec3(pos);

					if (ec->BrainType == BrainType::FOLLOW)
					{
						newPos = handleFollowBrain(pos, ballPosition, delta);
					}
					else if (ec->BrainType == BrainType::SHORT_SIGHTED_CENTER)
					{
						newPos = handleShortSightedCenterBrain(pos, ballPosition, delta);
					}
					else if (ec->BrainType == BrainType::LAZY)
					{
						newPos = handleLazyBrain(pos, ballPosition, ballVelocity, delta);
					}
					else
					{
						std::cout << "Unhandled brain type" << std::endl;
					}

					newPos.y = std::clamp(newPos.y, 0.0f, (float)(PongConstants::GameBoundsHeight - PongConstants::PaddleHeight));

					tc->SetPosition(newPos);
				}
			}
		}
	}

	glm::vec3 moveTowards(glm::vec3 target, glm::vec3 current, float delta)
	{
		glm::vec3 newPos = current;
		const auto maxMovement = delta * PongConstants::PaddleMoveSpeedBase;

		if (target.y > current.y)
		{
			if (current.y + maxMovement >= target.y)
			{
				newPos.y = target.y;
			}
			else
			{
				newPos.y = current.y + maxMovement;
			}
		}
		else if (target.y < current.y)
		{
			if (current.y - maxMovement <= target.y)
			{
				newPos.y = target.y;
			}
			else
			{
				newPos.y = current.y - maxMovement;
			}
		}

		return newPos;
	}

	glm::vec3 handleFollowBrain(glm::vec3 pos, glm::vec3 ballPosition, float delta)
	{
		// Just always move towards the ball
		return moveTowards(glm::vec3(pos.x, ballPosition.y, 0.0f), pos, delta);
	}

	glm::vec3 handleShortSightedCenterBrain(glm::vec3 pos, glm::vec3 ballPosition, float delta)
	{
		if (std::fabsf(pos.x - ballPosition.x) > PongConstants::GameBoundsWidth / 2.0f)
		{
			// Its more than half away so cant see it
			return moveTowards(glm::vec3(pos.x, PongConstants::GameBoundsHeight / 2.0f, 0.0f), pos, delta);
		}

		// Its close enough to see, move towards it
		return moveTowards(glm::vec3(pos.x, ballPosition.y, 0.0f), pos, delta);
	}
	glm::vec3 handleLazyBrain(glm::vec3 pos, glm::vec3 ballPosition, glm::vec3 ballVelocity, float delta)
	{
		if (ballVelocity.x < 0.0f)
		{
			// If the ball is going away, do nothing
			return pos;
		}

		// If the ball is coming back move towards it
		return moveTowards(glm::vec3(pos.x, ballPosition.y, 0.0f), pos, delta);
	}
}