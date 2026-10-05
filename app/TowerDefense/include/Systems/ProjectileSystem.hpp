#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <functional>

namespace tower
{
	class ProjectileSystem : public hl::System
	{
	public:
		explicit ProjectileSystem(hl::Scene& scene);
		void update(float delta) override;

		std::function<void()> onKill;

	private:
		hl::Scene& _scene;
	};
}
