#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace hur
{
	class SpriteClipSystem : public hl::System
	{
	public:
		explicit SpriteClipSystem(hl::Scene& scene);
		void update(float delta) override;

	private:
		hl::Scene& _scene;
	};
}
