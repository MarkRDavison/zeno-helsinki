#pragma once

#include <Entities/Tile.hpp>
#include <vector>

namespace drl
{

	struct TerrainRow
	{
		std::vector<Tile> leftTiles;
		std::vector<Tile> rightTiles;
	};

}
