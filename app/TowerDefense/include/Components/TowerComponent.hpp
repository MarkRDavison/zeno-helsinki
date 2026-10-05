#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class TowerComponent : public hl::Component
	{
	public:
		int x = 0;
		int z = 0;
	};
}
