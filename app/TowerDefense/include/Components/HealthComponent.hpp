#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class HealthComponent : public hl::Component
	{
	public:
		float max = 1.0f;
		float current = 1.0f;
	};
}
