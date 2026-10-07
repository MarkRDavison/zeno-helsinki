#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <string>
#include <unordered_map>

namespace tower
{
	class BlockerComponent : public hl::Component
	{
	public:
		int x = 0;
		int z = 0;
		int sizeX = 1;
		int sizeZ = 1;
		std::unordered_map<std::string, float> resist;
	};
}
