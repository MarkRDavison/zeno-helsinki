#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

namespace tower
{
	class WeaponCatalog;
	class ProjectileCatalog;

	class CreepFireSystem : public hl::System
	{
	public:
		// matchFireCooldown is towers only (TowerFireSystem). Creeps use catalog cooldown.
		CreepFireSystem(
			hl::Scene& scene,
			hl::ResourceManager& resourceManager,
			WeaponCatalog& weapons,
			ProjectileCatalog& projectiles);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
		hl::ResourceManager& _resourceManager;
		WeaponCatalog& _weapons;
		ProjectileCatalog& _projectiles;
	};
}
