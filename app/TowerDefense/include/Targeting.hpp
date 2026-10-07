#pragma once

#include <Components/TeamComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/glm.hpp>
#include <cmath>

namespace tower
{
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
}
