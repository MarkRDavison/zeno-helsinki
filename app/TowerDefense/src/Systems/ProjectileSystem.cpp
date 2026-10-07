#include <Systems/ProjectileSystem.hpp>
#include <Components/ProjectileComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/StatusListComponent.hpp>
#include <Combat.hpp>
#include <Status.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>

namespace tower
{
	namespace
	{
		float xzDistance(const glm::vec3& a, const glm::vec3& b)
		{
			const float dx = a.x - b.x;
			const float dz = a.z - b.z;
			return glm::length(glm::vec2(dx, dz));
		}

		float defenderResist(
			const CreepComponent* creep,
			const StatusListComponent* list,
			const StatusCatalog& statuses,
			std::string_view type)
		{
			const float innate = creep != nullptr ? resistOf(creep->resist, type) : 0.0f;
			if (list == nullptr)
			{
				return innate;
			}

			const float typeResistance = channelSum(
				list->instances,
				statuses,
				std::string(type) + "_resistance");
			const float weakness = channelSum(list->instances, statuses, "weakness");
			return effectiveResist(innate, typeResistance, weakness);
		}
	}

	ProjectileSystem::ProjectileSystem(
		hl::Scene& scene,
		const StatusCatalog& statuses,
		const StatusCategoryCatalog& categories) :
		_scene(scene),
		_statuses(statuses),
		_categories(categories)
	{
	}

	void ProjectileSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, ProjectileComponent>(ProjectileTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* shot = entity->GetComponent<ProjectileComponent>();
			auto* target = _scene.getEntity(shot->targetId);
			const bool targetLive = target != nullptr
				&& !_scene.isPendingRemoval(target->Id)
				&& target->HasComponent<hl::TransformComponent>();
			if (targetLive)
			{
				const glm::vec3 targetPos = target->GetComponent<hl::TransformComponent>()->GetPosition();
				shot->lastDest = glm::vec3(
					targetPos.x + shot->aimOffset.x,
					shot->y,
					targetPos.z + shot->aimOffset.y);
			}

			auto* transform = entity->GetComponent<hl::TransformComponent>();
			const glm::vec3 pos = transform->GetPosition();
			const glm::vec3 dest = shot->lastDest;
			const float distance = xzDistance(pos, dest);
			const float step = shot->speed * delta;
			if (distance <= shot->hitRadius || distance <= step)
			{
				_scene.removeEntity(entity->Id);
				if (!targetLive)
				{
					continue;
				}

				auto* health = target->GetComponent<HealthComponent>();
				auto* creep = target->GetComponent<CreepComponent>();
				auto* list = target->GetComponent<StatusListComponent>();
				if (health != nullptr)
				{
					const float resist = defenderResist(
						creep,
						list,
						_statuses,
						shot->damageType);
					const float dealt = outgoingDamage(shot->damage, 0.0f, 0.0f);
					applyHit(health->current, dealt, resist);
				}

				const bool dead = health == nullptr || isDead(health->current);
				if (dead)
				{
					_scene.removeEntity(target->Id);
					if (onKill)
					{
						onKill();
					}

					continue;
				}

				if (list != nullptr)
				{
					for (const auto& statusId : shot->statuses)
					{
						const auto* def = _statuses.find(statusId);
						const auto* category = def != nullptr
							? _categories.find(def->category)
							: nullptr;
						if (def == nullptr || category == nullptr)
						{
							continue;
						}

						applyStatus(
							list->instances,
							*def,
							*category,
							_statuses,
							list->nextSeq);
					}
				}

				continue;
			}

			const glm::vec3 dir = glm::normalize(glm::vec3(dest.x - pos.x, 0.0f, dest.z - pos.z));
			transform->SetPosition(glm::vec3(
				pos.x + dir.x * step,
				shot->y,
				pos.z + dir.z * step));
		}
	}
}
