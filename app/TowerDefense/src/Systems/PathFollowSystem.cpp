#include <Systems/PathFollowSystem.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/TeamComponent.hpp>
#include <PathBlock.hpp>
#include <Combat.hpp>
#include <Services/LevelCatalog.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <cmath>
#include <optional>

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

	namespace
	{
		struct PathStall
		{
			TileCoord tile{};
			int entityId = -1;
		};

		std::optional<PathStall> stallOnPath(
			hl::Scene& scene,
			LevelCatalog& level,
			const PathFollowComponent& follow)
		{
			const auto& waypoints = level.path(follow.pathName);
			for (auto* entity : scene.getEntitiesWithComponents<EntityComponent, TeamComponent>())
			{
				if (scene.isPendingRemoval(entity->Id) || !hasTeam(entity, Team::Neutral))
				{
					continue;
				}

				const auto* health = entity->GetComponent<HealthComponent>();
				if (health != nullptr && isDead(health->current))
				{
					continue;
				}

				const auto* placed = entity->GetComponent<EntityComponent>();
				const PathBlockFootprint footprint{
					placed->x,
					placed->z,
					placed->sizeX,
					placed->sizeZ
				};
				if (const auto tile = firstBlockedTile(waypoints, follow.fromIndex, footprint))
				{
					return PathStall{ *tile, entity->Id };
				}
			}

			return std::nullopt;
		}

		bool inRangeOfStall(
			LevelCatalog& level,
			hl::Entity* creep,
			const PathFollowComponent& follow,
			const PathStall& stall)
		{
			const auto& waypoints = level.path(follow.pathName);
			if (follow.fromIndex >= 0 && follow.fromIndex < static_cast<int>(waypoints.size()))
			{
				const auto& here = waypoints[static_cast<std::size_t>(follow.fromIndex)];
				if (here.x == stall.tile.x && here.z == stall.tile.z)
				{
					return true;
				}
			}

			const auto* body = creep->GetComponent<CreepComponent>();
			const float range = body != nullptr ? body->range : 0.0f;
			const auto* transform = creep->GetComponent<hl::TransformComponent>();
			const glm::vec3 center = level.tileCenter(stall.tile.x, stall.tile.z);
			return shouldStall(transform->GetPosition(), center, range);
		}

		std::optional<PathStall> activeStall(
			hl::Scene& scene,
			LevelCatalog& level,
			hl::Entity* creep,
			PathFollowComponent& follow)
		{
			const auto stall = stallOnPath(scene, level, follow);
			if (!stall.has_value() || !inRangeOfStall(level, creep, follow, *stall))
			{
				follow.stalledEntityId = -1;
				return std::nullopt;
			}

			follow.stalledEntityId = stall->entityId;
			return stall;
		}
	}

	void PathFollowSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<
			hl::TransformComponent,
			PathFollowComponent,
			TeamComponent>())
		{
			if (_scene.isPendingRemoval(entity->Id) || !hasTeam(entity, Team::Creep))
			{
				continue;
			}

			auto* follow = entity->GetComponent<PathFollowComponent>();
			auto* transform = entity->GetComponent<hl::TransformComponent>();
			const auto& waypoints = _level.path(follow->pathName);
			const int waypointCount = static_cast<int>(waypoints.size());

			if (activeStall(_scene, _level, entity, *follow))
			{
				continue;
			}

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
					if (activeStall(_scene, _level, entity, *follow))
					{
						follow->t = 0.0f;
						const auto& tile = waypoints[static_cast<std::size_t>(follow->fromIndex)];
						transform->SetPosition(_level.tileCenter(tile.x, tile.z));
						break;
					}

					const auto& last = waypoints[static_cast<std::size_t>(waypointCount - 1)];
					transform->SetPosition(_level.tileCenter(last.x, last.z));
					leak(entity);
					break;
				}

				if (activeStall(_scene, _level, entity, *follow))
				{
					follow->t = 0.0f;
					const auto& tile = waypoints[static_cast<std::size_t>(follow->fromIndex)];
					transform->SetPosition(_level.tileCenter(tile.x, tile.z));
					break;
				}
			}

			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			if (activeStall(_scene, _level, entity, *follow))
			{
				continue;
			}

			if (follow->fromIndex >= waypointCount - 1)
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
