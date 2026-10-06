#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <Services/TowerCatalog.hpp>

namespace tower
{
	class TowerFireSystem : public hl::System
	{
	public:
		TowerFireSystem(
			hl::Scene& scene,
			hl::ResourceManager& resourceManager,
			TowerCatalog& towers);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
		hl::ResourceManager& _resourceManager;
		TowerCatalog& _towers;
	};
}
