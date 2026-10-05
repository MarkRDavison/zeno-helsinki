#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

namespace tower
{
	class TowerFireSystem : public hl::System
	{
	public:
		TowerFireSystem(hl::Scene& scene, hl::ResourceManager& resourceManager);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
		hl::ResourceManager& _resourceManager;
	};
}
