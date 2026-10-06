#include <Systems/PathFollowSystem.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Services/LevelCatalog.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <cmath>

namespace tower
{
	PathFollowSystem::PathFollowSystem(hl::Scene& scene, LevelCatalog& level) :
		_scene(scene),
		_level(level)
	{
	}

	void PathFollowSystem::leak(hl::Entity* entity)
	{
		if (onLeak)
		{
			onLeak();
		}

		_scene.removeEntity(entity->Id);
	}

	void PathFollowSystem::update(float delta)
	{
		const auto& waypoints = _level.path();
		const int waypointCount = static_cast<int>(waypoints.size());

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
				leak(entity);
				continue;
			}

			const auto from = waypoints[static_cast<std::size_t>(follow->fromIndex)];
			const auto to = waypoints[static_cast<std::size_t>(follow->fromIndex + 1)];
			const glm::vec3 a = _level.tileCenter(from.x, from.z);
			const glm::vec3 b = _level.tileCenter(to.x, to.z);
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
					const auto& last = waypoints[static_cast<std::size_t>(waypointCount - 1)];
					transform->SetPosition(_level.tileCenter(last.x, last.z));
					leak(entity);
					break;
				}
			}

			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			const auto fromWp = waypoints[static_cast<std::size_t>(follow->fromIndex)];
			const auto toWp = waypoints[static_cast<std::size_t>(follow->fromIndex + 1)];
			const glm::vec3 start = _level.tileCenter(fromWp.x, fromWp.z);
			const glm::vec3 end = _level.tileCenter(toWp.x, toWp.z);
			const glm::vec3 dir = end - start;
			transform->SetPosition(glm::mix(start, end, follow->t));
			transform->SetRotation(glm::vec3(0.0f, glm::degrees(std::atan2(dir.x, dir.z)), 0.0f));
		}
	}
}
