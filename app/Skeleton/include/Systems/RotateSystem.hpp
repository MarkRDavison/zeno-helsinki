#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace sk
{
	class RotateSystem : public hl::System
	{
	public:
		explicit RotateSystem(hl::Scene& scene);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
		float _angle = 0.0f;
	};
}
