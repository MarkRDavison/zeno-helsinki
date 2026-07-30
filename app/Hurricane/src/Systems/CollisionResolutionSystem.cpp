#include <Systems/CollisionResolutionSystem.hpp>
#include <Events/CollisionEvent.hpp>
#include <iostream>

namespace hur
{

	CollisionResolutionSystem::CollisionResolutionSystem(
		hl::EventBus& eventBus,
		hl::Scene& scene
	) :
		_eventBus(eventBus),
		_scene(scene)
	{
		_eventBus.AddListener(this);
	}
	CollisionResolutionSystem::~CollisionResolutionSystem()
	{
		_eventBus.RemoveListener(this);
	}

	void CollisionResolutionSystem::update(float delta)
	{

	}

	void CollisionResolutionSystem::OnEvent(const hl::Event& event)
	{
		if (auto ce = dynamic_cast<const CollisionEvent*>(&event))
		{
			auto entityA = _scene.getEntity(ce->getEntityAId());
			auto entityB = _scene.getEntity(ce->getEntityBId());

			if (entityA == nullptr || entityB == nullptr)
			{
				return;
			}

			auto projectileIsA = entityA->HasTag("PROJECTILE");
			auto projectileIsB = entityB->HasTag("PROJECTILE");

			if (projectileIsA)
			{
				_scene.removeEntity(entityA->Id);

				// TODO: DAMAGE/HEALTH
				_scene.removeEntity(entityB->Id);
			}
			else if (projectileIsB)
			{
				_scene.removeEntity(entityB->Id);

				// TODO: DAMAGE/HEALTH
				_scene.removeEntity(entityA->Id);
			}
			else
			{
				return;
			}
		}
	}
}