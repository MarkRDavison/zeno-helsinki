#pragma once

#include <helsinki/System/glm.hpp>
#include <cmath>

namespace drl
{

	inline glm::ivec2 pixelToTile(glm::vec2 mouse, float originX, float originY, float tileSize)
	{
		return glm::ivec2(
			static_cast<int>(std::floor((mouse.x - originX) / tileSize)),
			static_cast<int>(std::floor((mouse.y - originY) / tileSize)));
	}

}
