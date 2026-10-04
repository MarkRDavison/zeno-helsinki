#include <Systems/CollisionResolutionSystem.hpp>
#include <Events/CollisionEvent.hpp>
#include <Events/MissileExplodeEvent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/PickupComponent.hpp>
#include <Components/PickupKind.hpp>
#include <Components/PlayerLoadoutComponent.hpp>
#include <Components/ProjectileComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>

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

			if (_scene.isPendingRemoval(entityA->Id) || _scene.isPendingRemoval(entityB->Id))
			{
				return;
			}

			const auto pickupIsA = entityA->HasTag("PICKUP");
			const auto pickupIsB = entityB->HasTag("PICKUP");
			if (pickupIsA || pickupIsB)
			{
				const auto aIsPlayer = entityA->HasTag("PLAYER");
				const auto bIsPlayer = entityB->HasTag("PLAYER");

				hl::Entity* player = nullptr;
				hl::Entity* pickup = nullptr;
				if (pickupIsA && bIsPlayer)
				{
					pickup = entityA;
					player = entityB;
				}
				else if (pickupIsB && aIsPlayer)
				{
					pickup = entityB;
					player = entityA;
				}

				if (player == nullptr || pickup == nullptr)
				{
					return;
				}

				if (auto* pickupComp = pickup->GetComponent<PickupComponent>())
				{
					const PickupKind kind = pickupKindFromId(pickupComp->pickupId);
					if (auto* loadout = player->GetComponent<PlayerLoadoutComponent>())
					{
						applyPickup(
							*loadout,
							player->GetComponent<WeaponComponent>(),
							kind);
					}
				}

				_scene.removeEntity(pickup->Id);
				return;
			}

			auto projectileIsA = entityA->HasTag("PROJECTILE"); // TODO: CONSTANTS
			auto projectileIsB = entityB->HasTag("PROJECTILE");
			const auto missileIsA = entityA->HasTag("MISSILE");
			const auto missileIsB = entityB->HasTag("MISSILE");

			if (missileIsA || missileIsB)
			{
				hl::Entity* missile = missileIsA ? entityA : entityB;
				hl::Entity* other = missileIsA ? entityB : entityA;
				if (!other->HasTag("ENEMY"))
				{
					return;
				}

				glm::vec3 explodeAt{};
				if (auto* transform = missile->GetComponent<hl::TransformComponent>())
				{
					explodeAt = transform->GetPosition();
				}

				_scene.removeEntity(missile->Id);
				_eventBus.PublishEvent(MissileExplodeEvent(explodeAt));
				return;
			}

			if (projectileIsA)
			{
				_scene.removeEntity(entityA->Id);

				int damage = 3;
				if (auto* projectile = entityA->GetComponent<ProjectileComponent>())
				{
					damage = projectile->damage;
				}
				const DeathType deathType =
					entityB->HasTag("PLAYER") ? DeathType::NO_DROP : DeathType::DROP;
				applyDamageToEntity(entityB, damage, deathType);

			}
			else if (projectileIsB)
			{
				_scene.removeEntity(entityB->Id);

				int damage = 3;
				if (auto* projectile = entityB->GetComponent<ProjectileComponent>())
				{
					damage = projectile->damage;
				}
				const DeathType deathType =
					entityA->HasTag("PLAYER") ? DeathType::NO_DROP : DeathType::DROP;
				applyDamageToEntity(entityA, damage, deathType);
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
		if (entity->HasTag("PLAYER"))
		{
			if (auto* loadout = entity->GetComponent<PlayerLoadoutComponent>())
			{
				if (loadout->shieldLayers > 0)
				{
					--loadout->shieldLayers;
					return;
				}
			}
		}

		auto aHealth = entity->GetComponent<HealthComponent>();
		if (aHealth == nullptr)
		{
			return;
		}

		const auto currentAHealth = aHealth->getCurrentHealth();

		aHealth->setCurrentHealth(std::max(currentAHealth - damage, 0));

		if (aHealth->getCurrentHealth() <= 0)
		{
			_eventBus.PublishEvent(EntityDeathEvent(entity->Id, type));
		}
	}
}