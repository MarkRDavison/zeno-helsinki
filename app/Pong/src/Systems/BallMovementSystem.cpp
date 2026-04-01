#include <Systems/BallMovementSystem.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <EngineCandidates/CollisionData.hpp>
#include <Events/PointScoredEvent.hpp>
#include <Components/EntityComponent.hpp>
#include <PongConstants.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

namespace pong
{

	BallMovementSystem::BallMovementSystem(
		hl::InputManager& inputManager,
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_inputManager(inputManager),
		_eventBus(eventBus),
		_scene(scene)
	{

	}

	void BallMovementSystem::update(float delta)
	{
		auto b = _scene.getEntity("Ball"); // TODO: Handle multiple balls

		auto tc = b->GetComponent<hl::TransformComponent>();
		auto kc = b->GetComponent<hl::KinematicComponent>();
		auto ec = b->GetComponent<pong::EntityComponent>();

		const auto oldPos = tc->GetPosition();
		const auto offset = kc->velocity * delta;

		auto newPos = tc->GetPosition() + offset;

		const auto top = 0.0f;
		const auto left = 0.0f;
		const auto bot = PongConstants::GameBoundsHeight - PongConstants::BallSize;
		const auto right = PongConstants::GameBoundsWidth - PongConstants::BallSize;

		for (const auto& e : _scene.getEntities())
		{
			if ((e->HasTag("PADDLE") || e->HasTag("WALL")) &&
				e->HasComponents<hl::TransformComponent>())
			{
				auto ballCollisionData = CollisionData
				{
					.X = newPos.x,
					.Y = newPos.y,
					.W = PongConstants::BallSize,
					.H = PongConstants::BallSize,
					.vX = kc->velocity.x,
					.vY = kc->velocity.y
				};

				auto eTc = e->GetComponent<hl::TransformComponent>();

				// TODO: Record bounds on entity???
				auto entityCollisionData = CollisionData
				{
					.X = eTc->GetPosition().x,
					.Y = eTc->GetPosition().y,
					.W = (float)(e->HasTag("PADDLE") ? PongConstants::PaddleWidth : PongConstants::GameBoundsWidth),
					.H = (float)(e->HasTag("PADDLE") ? PongConstants::PaddleHeight : PongConstants::GameBoundsWallWidth),
					.vX = 0.0f,
					.vY = 0.0f
				};

				if (ballCollisionData.Collides(entityCollisionData))
				{
					// TODO: Need this to be way better and handle collisions properly
					// Need to actually calculate how deep the collision was etc and 
					// apply reverse the collision depth, not just revert pos and invert vel
					// Cos this means it looks like the paddle/walls have a small 'force field' effect
					// Also if you collide with the bottom of a paddle that behaves like you hit the front
					if (e->HasTag("PADDLE"))
					{
						newPos.x = oldPos.x;
						kc->velocity.x *= -1.1f;
					}
					else
					{
						newPos.y = oldPos.y;
						kc->velocity.y *= -1.1f;
					}
				}

			}
		}

		if (newPos.x < left)
		{
			kc->velocity = {};

			PointScoredEvent e(2);
			_eventBus.PublishEvent(e);
		}

		if (newPos.x > right)
		{
			kc->velocity = {};

			PointScoredEvent e(1);
			_eventBus.PublishEvent(e);
		}		

		tc->SetPosition(newPos);
	}

}