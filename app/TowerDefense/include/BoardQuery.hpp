#pragma once

#include <Components/EntityComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace tower
{
	inline bool tileOccupied(const hl::Scene& scene, int tx, int tz)
	{
		for (auto* entity : scene.getEntitiesByTag(TowerTag))
		{
			if (scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			const auto* tower = entity->GetComponent<TowerComponent>();
			if (tower != nullptr && tower->x == tx && tower->z == tz)
			{
				return true;
			}
		}

		for (auto* entity : scene.getEntitiesWithComponents<EntityComponent>())
		{
			if (scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			const auto* placed = entity->GetComponent<EntityComponent>();
			if (placed != nullptr
				&& tx >= placed->x && tx < placed->x + placed->sizeX
				&& tz >= placed->z && tz < placed->z + placed->sizeZ)
			{
				return true;
			}
		}

		return false;
	}

	inline bool tileUnbuildable(const LevelCatalog& level, const hl::Scene& scene, int tx, int tz)
	{
		return !level.isOnBoard(tx, tz) || level.isPathTile(tx, tz) || tileOccupied(scene, tx, tz);
	}
}
