#include <Systems/PathFollowSystem.hpp>
#include <Components/PathFollowComponent.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <cmath>

namespace tower
{
	PathFollowSystem::PathFollowSystem(hl::Scene& scene) :
		_scene(scene)
	{
	}

	void PathFollowSystem::update(float delta)
	{
		constexpr int waypointCount = static_cast<int>(sizeof(PathWaypoints) / sizeof(PathWaypoints[0]));

		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, PathFollowComponent>(CreepTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* follow = entity->GetComponent<PathFollowComponent>();
			auto* transform = entity->GetComponent<hl::TransformComponent>();

			if (follow->fromIndex >= waypointCount - 1)
			{
				_scene.removeEntity(entity->Id);
				continue;
			}

			const auto from = PathWaypoints[follow->fromIndex];
			const auto to = PathWaypoints[follow->fromIndex + 1];
			const glm::vec3 a = tileCenter(from.x, from.z);
			const glm::vec3 b = tileCenter(to.x, to.z);
			const float length = glm::length(b - a);
			if (length < 1e-6f)
			{
				follow->fromIndex += 1;
				follow->t = 0.0f;
				continue;
			}

			follow->t += (follow->speed * delta) / length;
			while (follow->t >= 1.0f && follow->fromIndex < waypointCount - 1)
			{
				follow->t -= 1.0f;
				follow->fromIndex += 1;
				if (follow->fromIndex >= waypointCount - 1)
				{
					transform->SetPosition(tileCenter(
						PathWaypoints[waypointCount - 1].x,
						PathWaypoints[waypointCount - 1].z));
					_scene.removeEntity(entity->Id);
					break;
				}
			}

			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			const auto fromWp = PathWaypoints[follow->fromIndex];
			const auto toWp = PathWaypoints[follow->fromIndex + 1];
			const glm::vec3 start = tileCenter(fromWp.x, fromWp.z);
			const glm::vec3 end = tileCenter(toWp.x, toWp.z);
			const glm::vec3 dir = end - start;
			transform->SetPosition(glm::mix(start, end, follow->t));
			transform->SetRotation(glm::vec3(0.0f, glm::degrees(std::atan2(dir.x, dir.z)), 0.0f));
		}
	}
}
