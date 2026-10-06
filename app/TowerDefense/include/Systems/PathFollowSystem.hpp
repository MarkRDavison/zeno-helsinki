#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <functional>

namespace tower
{
	class LevelCatalog;

	class PathFollowSystem : public hl::System
	{
	public:
		PathFollowSystem(hl::Scene& scene, LevelCatalog& level);
		void update(float delta) override;

		std::function<void()> onLeak;

	private:
		void leak(hl::Entity* entity);

		hl::Scene& _scene;
		LevelCatalog& _level;
	};
}
