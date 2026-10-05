#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class TileComponent : public hl::Component
	{
	public:
		int x = 0;
		int z = 0;
		bool path = false;
	};
}
