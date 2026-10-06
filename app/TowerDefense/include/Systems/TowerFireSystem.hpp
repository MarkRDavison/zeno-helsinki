#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/MatchContext.hpp>

namespace tower
{
	class LevelCatalog;
	class WeaponCatalog;
	class ProjectileCatalog;

	class TowerFireSystem : public hl::System
	{
	public:
		TowerFireSystem(
			hl::Scene& scene,
			hl::ResourceManager& resourceManager,
			TowerCatalog& towers,
			WeaponCatalog& weapons,
			ProjectileCatalog& projectiles,
			LevelCatalog& level,
			MatchContext& match);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
		hl::ResourceManager& _resourceManager;
		TowerCatalog& _towers;
		WeaponCatalog& _weapons;
		ProjectileCatalog& _projectiles;
		LevelCatalog& _level;
		MatchContext& _match;
	};
}
