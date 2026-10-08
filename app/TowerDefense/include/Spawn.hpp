#pragma once

#include <helsinki/Engine/Scene/Scene.hpp>
#include <string_view>

namespace hl
{
	class ResourceManager;
	class Entity;
}

namespace tower
{
	struct TowerDef;
	struct CreepDef;
	class LevelCatalog;
	class EntityCatalog;

	hl::Entity* spawnTower(
		hl::Scene& scene,
		const TowerDef& def,
		const LevelCatalog& level,
		int x,
		int z,
		hl::ResourceManager* resources = nullptr);

	hl::Entity* spawnCreep(
		hl::Scene& scene,
		const CreepDef& def,
		const LevelCatalog& level,
		std::string_view pathName,
		hl::ResourceManager* resources = nullptr);

	void spawnEntities(
		hl::Scene& scene,
		const LevelCatalog& level,
		const EntityCatalog& entities,
		hl::ResourceManager* resources = nullptr);
}
