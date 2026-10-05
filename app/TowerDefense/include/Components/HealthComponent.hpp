#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class HealthComponent : public hl::Component
	{
	public:
		int max = 1;
		int current = 1;
	};
}
