#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace hur
{
	class ProjectileComponent : public hl::Component
	{
	public:
		int damage{ 3 };
	};
}
