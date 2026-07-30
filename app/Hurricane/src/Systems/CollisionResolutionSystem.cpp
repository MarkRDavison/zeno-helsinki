#include <Systems/CollisionResolutionSystem.hpp>
#include <Events/CollisionEvent.hpp>
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
		// TODO: What happens if projectile hits multiple enemies in one frame?
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

				const int damage = 3;
				applyDamageToEntity(entityB, damage, DeathType::DROP);

			}
			else if (projectileIsB)
			{
				_scene.removeEntity(entityB->Id);

				const int damage = 3;
				applyDamageToEntity(entityA, damage, DeathType::DROP);
			}
			else
			{
				const auto aIsPlayer = entityA->HasTag("PLAYER");// TODO: CONSTANT
				const auto bIsPlayer = entityB->HasTag("PLAYER");// TODO: CONSTANT

				const auto aIsEnemy = entityA->HasTag("ENEMY");// TODO: CONSTANT
				const auto bIsEnemy = entityB->HasTag("ENEMY");// TODO: CONSTANT

				if (aIsPlayer && bIsEnemy)
				{
					const int damage = 5;
					applyDamageToEntity(entityA, damage, DeathType::NO_DROP); // PLAYER DOESNT DROP

					_eventBus.PublishEvent(EntityDeathEvent(entityB->Id, DeathType::NO_DROP));
				}
				else if (bIsPlayer && aIsEnemy)
				{
					const int damage = 5;
					applyDamageToEntity(entityB, damage, DeathType::NO_DROP); // PLAYER DOESNT DROP

					_eventBus.PublishEvent(EntityDeathEvent(entityA->Id, DeathType::NO_DROP));
				}
			}
		}
	}

	void CollisionResolutionSystem::applyDamageToEntity(hl::Entity* entity, int damage, DeathType type)
	{
		auto aHealth = entity->GetComponent<HealthComponent>();

		const auto currentAHealth = aHealth->getCurrentHealth();

		aHealth->setCurrentHealth(std::max(currentAHealth - damage, 0));

		if (aHealth->getCurrentHealth() <= 0)
		{
			_eventBus.PublishEvent(EntityDeathEvent(entity->Id, type));
		}
	}
}