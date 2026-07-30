#include <Systems/CollisionResolutionSystem.hpp>
#include <Events/CollisionEvent.hpp>
#include <Events/EntityDeathEvent.hpp>
#include <Components/HealthComponent.hpp>

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

			auto projectileIsA = entityA->HasTag("PROJECTILE"); // TODO: CONSTANTS
			auto projectileIsB = entityB->HasTag("PROJECTILE");

			if (projectileIsA)
			{
				_scene.removeEntity(entityA->Id);

				auto bHealth = entityB->GetComponent<HealthComponent>();

				const auto currentBHealth = bHealth->getCurrentHealth();
				const int damage = 3;

				bHealth->setCurrentHealth(std::max(currentBHealth - damage, 0));

				if (bHealth->getCurrentHealth() <= 0)
				{
					_eventBus.PublishEvent(EntityDeathEvent(entityB->Id));
				}

			}
			else if (projectileIsB)
			{
				_scene.removeEntity(entityB->Id);

				auto aHealth = entityA->GetComponent<HealthComponent>();

				const auto currentAHealth = aHealth->getCurrentHealth();
				const int damage = 3;

				aHealth->setCurrentHealth(std::max(currentAHealth - damage, 0));

				if (aHealth->getCurrentHealth() <= 0)
				{
					_eventBus.PublishEvent(EntityDeathEvent(entityA->Id));
				}
			}
			else
			{
				return;
			}
		}
	}
}