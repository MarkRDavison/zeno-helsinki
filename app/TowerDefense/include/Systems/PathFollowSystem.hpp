#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <functional>

namespace tower
{
	class PathFollowSystem : public hl::System
	{
	public:
		explicit PathFollowSystem(hl::Scene& scene);
		void update(float delta) override;

		std::function<void()> onLeak;

	private:
		void leak(hl::Entity* entity);

		hl::Scene& _scene;
	};
}
