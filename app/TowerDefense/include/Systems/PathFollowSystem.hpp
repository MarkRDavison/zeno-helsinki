#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace tower
{
	class PathFollowSystem : public hl::System
	{
	public:
		explicit PathFollowSystem(hl::Scene& scene);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
	};
}
