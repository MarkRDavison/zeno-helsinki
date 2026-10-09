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

	inline glm::vec4 atlasUvRect(int column, int row, float tileSize, float texSize, float insetTexels = 0.5f)
	{
		return glm::vec4(
			(tileSize * static_cast<float>(column) + insetTexels) / texSize,
			(tileSize * static_cast<float>(row) + insetTexels) / texSize,
			(tileSize * static_cast<float>(column + 1) - insetTexels) / texSize,
			(tileSize * static_cast<float>(row + 1) - insetTexels) / texSize);
	}

}
