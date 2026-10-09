#pragma once

#include <Entities/TerrainRow.hpp>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace drl
{

	struct TerrainData
	{
		std::vector<TerrainRow> rows;
		int shaftLevel{ -1 };

		bool tileExists(int level, int column) const
		{
			if (column == 0)
			{
				return false;
			}
			const unsigned xCoord = static_cast<unsigned>(std::abs(column) - 1);
			if (level >= static_cast<int>(rows.size()))
			{
				return false;
			}
			const TerrainRow& row = rows[static_cast<unsigned>(level)];

			if (column > 0)
			{
				return static_cast<unsigned>(row.rightTiles.size()) > xCoord;
			}
			return static_cast<unsigned>(row.leftTiles.size()) > xCoord;
		}

		const Tile& getTile(int level, int column) const
		{
			const unsigned xCoord = static_cast<unsigned>(std::abs(column)) - (column == 0 ? 0 : 1);
			if (column < 0)
			{
				return rows[static_cast<unsigned>(level)].leftTiles[xCoord];
			}
			if (column > 0)
			{
				return rows[static_cast<unsigned>(level)].rightTiles[xCoord];
			}
			throw std::runtime_error("Cannot retrieve shaft Tile");
		}

		Tile& getTile(int level, int column)
		{
			const unsigned xCoord = static_cast<unsigned>(std::abs(column)) - (column == 0 ? 0 : 1);
			if (column < 0)
			{
				return rows[static_cast<unsigned>(level)].leftTiles[xCoord];
			}
			if (column > 0)
			{
				return rows[static_cast<unsigned>(level)].rightTiles[xCoord];
			}
			throw std::runtime_error("Cannot retrieve shaft Tile");
		}
	};

}
