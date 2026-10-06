#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class BlockerComponent : public hl::Component
	{
	public:
		int x = 0;
		int z = 0;
		int sizeX = 1;
		int sizeZ = 1;
	};
}
