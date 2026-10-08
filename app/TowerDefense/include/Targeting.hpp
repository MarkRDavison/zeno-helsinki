#pragma once

#include <Components/HealthComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/glm.hpp>
#include <cmath>

namespace tower
{
	inline constexpr int TowerFocusNone = -1;

	inline float targetingXzDistance(const glm::vec3& a, const glm::vec3& b)
	{
		const float dx = a.x - b.x;
		const float dz = a.z - b.z;
		return std::sqrt(dx * dx + dz * dz);
	}

	inline hl::Entity* nearestInRangeWithTeam(
		hl::Scene& scene,
		const glm::vec3& from,
		float range,
		Team team)
	{
		hl::Entity* nearest = nullptr;
		float best = range;
		for (auto* entity : scene.getEntitiesWithComponents<hl::TransformComponent, TeamComponent>())
		{
			if (scene.isPendingRemoval(entity->Id) || !hasTeam(entity, team))
			{
				continue;
			}

			const float distance = targetingXzDistance(
				from,
				entity->GetComponent<hl::TransformComponent>()->GetPosition());
			if (distance <= best)
			{
				best = distance;
				nearest = entity;
			}
		}

		return nearest;
	}

	inline bool isValidTowerFocus(
		hl::Scene& scene,
		const glm::vec3& from,
		float range,
		const hl::Entity* entity)
	{
		if (entity == nullptr
			|| scene.isPendingRemoval(entity->Id)
			|| !hasTeam(entity, Team::Neutral)
			|| entity->GetComponent<HealthComponent>() == nullptr)
		{
			return false;
		}

		const auto* transform = entity->GetComponent<hl::TransformComponent>();
		if (transform == nullptr)
		{
			return false;
		}

		return targetingXzDistance(from, transform->GetPosition()) <= range;
	}

	inline bool tryAssignTowerFocus(
		hl::Scene& scene,
		const glm::vec3& from,
		float range,
		hl::Entity* candidate,
		int& focusEntityId)
	{
		if (!isValidTowerFocus(scene, from, range, candidate))
		{
			return false;
		}

		focusEntityId = candidate->Id;
		return true;
	}

	inline hl::Entity* resolveTowerFireTarget(
		hl::Scene& scene,
		const glm::vec3& from,
		float range,
		int& focusEntityId)
	{
		if (focusEntityId != TowerFocusNone)
		{
			auto* focused = scene.getEntity(focusEntityId);
			if (isValidTowerFocus(scene, from, range, focused))
			{
				return focused;
			}

			focusEntityId = TowerFocusNone;
		}

		return nearestInRangeWithTeam(scene, from, range, Team::Creep);
	}
}
