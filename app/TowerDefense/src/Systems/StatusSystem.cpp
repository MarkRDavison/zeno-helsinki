#include <Systems/StatusSystem.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/StatusListComponent.hpp>
#include <Combat.hpp>
#include <Status.hpp>
#include <SceneCatalog.hpp>
#include <iostream>
#include <string>

namespace tower
{
	namespace
	{
		void recomputeCreep(
			hl::Entity& entity,
			const StatusCatalog& statuses)
		{
			auto* follow = entity.GetComponent<PathFollowComponent>();
			auto* creep = entity.GetComponent<CreepComponent>();
			auto* health = entity.GetComponent<HealthComponent>();
			auto* list = entity.GetComponent<StatusListComponent>();
			if (follow == nullptr || creep == nullptr || list == nullptr)
			{
				return;
			}

			const float slow = channelSum(list->instances, statuses, "speed");
			const auto clampedSlow = clampUnit(slow);
			if (clampedSlow.warned)
			{
				std::clog << "tower: slow magnitude sum clamped to 0..1\n";
			}

			follow->speed = follow->baseSpeed * (1.0f - clampedSlow.value);

			if (health == nullptr)
			{
				return;
			}

			const float healthSum = channelSum(list->instances, statuses, "health");
			health->max = creep->baseHealth * (1.0f + healthSum);
			if (health->current > health->max)
			{
				health->current = health->max;
			}
		}
	}

	StatusSystem::StatusSystem(hl::Scene& scene, const StatusCatalog& statuses) :
		_scene(scene),
		_statuses(statuses)
	{
	}

	void StatusSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<StatusListComponent>(CreepTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* list = entity->GetComponent<StatusListComponent>();
			auto* health = entity->GetComponent<HealthComponent>();
			auto* creep = entity->GetComponent<CreepComponent>();
			const auto ticks = tickDots(list->instances, _statuses, delta);
			if (health != nullptr && creep != nullptr)
			{
				for (const auto& tick : ticks)
				{
					const float innate = resistOf(creep->resist, tick.damageType);
					const float typeResistance = channelSum(
						list->instances,
						_statuses,
						tick.damageType + std::string("_resistance"));
					const float weakness = channelSum(list->instances, _statuses, "weakness");
					applyHit(
						health->current,
						tick.damage,
						effectiveResist(innate, typeResistance, weakness));
				}

				if (isDead(health->current))
				{
					_scene.removeEntity(entity->Id);
					if (onKill)
					{
						onKill();
					}

					continue;
				}
			}

			recomputeCreep(*entity, _statuses);
		}
	}
}
