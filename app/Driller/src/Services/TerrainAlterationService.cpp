#include <Services/TerrainAlterationService.hpp>
#include <cmath>
#include <stdexcept>

namespace drl
{

	TerrainAlterationService::TerrainAlterationService(TerrainData& terrainData)
		: _terrainData(terrainData)
	{
	}

	bool TerrainAlterationService::digShaft(int level)
	{
		if (_terrainData.shaftLevel == level - 1)
		{
			_terrainData.shaftLevel = level;
			if (_terrainData.shaftLevel >= static_cast<int>(_terrainData.rows.size()))
			{
				_terrainData.rows.emplace_back();
			}
			return true;
		}

		return false;
	}

	namespace
	{
		Tile* getTileInternal(TerrainData& terrainData, int level, int column)
		{
			if (column == 0)
			{
				return nullptr;
			}

			while (static_cast<int>(terrainData.rows.size()) <= level)
			{
				terrainData.rows.emplace_back();
			}

			if (static_cast<int>(terrainData.rows.size()) > level)
			{
				TerrainRow& row = terrainData.rows[static_cast<unsigned>(level)];
				const unsigned xCoord = static_cast<unsigned>(std::abs(column) - (column == 0 ? 0 : 1));
				if (column > 0)
				{
					while (row.rightTiles.size() <= xCoord)
					{
						row.rightTiles.emplace_back();
					}
					return &row.rightTiles[xCoord];
				}

				if (column < 0)
				{
					while (row.leftTiles.size() <= xCoord)
					{
						row.leftTiles.emplace_back();
					}
					return &row.leftTiles[xCoord];
				}
			}

			return nullptr;
		}
	}

	bool TerrainAlterationService::digTile(int level, int column)
	{
		if (column == 0)
		{
			return false;
		}

		while (static_cast<int>(_terrainData.rows.size()) <= level)
		{
			_terrainData.rows.emplace_back();
		}

		TerrainRow& row = _terrainData.rows[static_cast<unsigned>(level)];
		const unsigned xCoord = static_cast<unsigned>(std::abs(column) - (column == 0 ? 0 : 1));
		if (column > 0)
		{
			if (row.rightTiles.size() == xCoord)
			{
				Tile& t = row.rightTiles.emplace_back();
				t.jobReserved = false;
				t.dugOut = true;
				return true;
			}
			if (row.rightTiles.size() > xCoord)
			{
				Tile& t = row.rightTiles[xCoord];
				if (t.dugOut)
				{
					return false;
				}
				t.jobReserved = false;
				t.dugOut = true;
				return true;
			}

			return false;
		}

		if (column < 0)
		{
			if (row.leftTiles.size() == xCoord)
			{
				Tile& t = row.leftTiles.emplace_back();
				t.jobReserved = false;
				t.dugOut = true;
				return true;
			}
			if (row.leftTiles.size() > xCoord)
			{
				Tile& t = row.leftTiles[xCoord];
				if (t.dugOut)
				{
					return false;
				}
				t.jobReserved = false;
				t.dugOut = true;
				return true;
			}

			return false;
		}

		return false;
	}

	bool TerrainAlterationService::createTile(int levelMin, int levelMax, int columnMin, int columnMax)
	{
		if (columnMin < 0 && columnMax < 0)
		{
			for (int level = levelMin; level <= levelMax; ++level)
			{
				for (int column = columnMin; column >= columnMax; --column)
				{
					getTileInternal(_terrainData, level, column);
				}
			}
		}
		else
		{
			for (int level = levelMin; level <= levelMax; ++level)
			{
				for (int column = columnMin; column <= columnMax; ++column)
				{
					getTileInternal(_terrainData, level, column);
				}
			}
		}

		return true;
	}

	bool TerrainAlterationService::doesTileExist(int level, int column) const
	{
		if (column == 0)
		{
			return false;
		}
		const unsigned xCoord = static_cast<unsigned>(std::abs(column) - 1);
		if (level < 0 || level >= static_cast<int>(_terrainData.rows.size()))
		{
			return false;
		}
		const TerrainRow& row = _terrainData.rows[static_cast<unsigned>(level)];

		if (column > 0)
		{
			return static_cast<unsigned>(row.rightTiles.size()) > xCoord;
		}
		return static_cast<unsigned>(row.leftTiles.size()) > xCoord;
	}

	Tile& TerrainAlterationService::getTile(int level, int column)
	{
		if (level < 0)
		{
			throw std::runtime_error("Cannot retrieve above ground TerrainTile");
		}

		if (column != 0)
		{
			if (!doesTileExist(level, column))
			{
				createTile(level, level, column, column);
			}

			const unsigned xCoord = static_cast<unsigned>(std::abs(column)) - (column == 0 ? 0 : 1);
			if (column < 0)
			{
				return _terrainData.rows[static_cast<unsigned>(level)].leftTiles[xCoord];
			}
			if (column > 0)
			{
				return _terrainData.rows[static_cast<unsigned>(level)].rightTiles[xCoord];
			}
		}

		throw std::runtime_error("Cannot retrieve shaft TerrainTile");
	}

	const Tile& TerrainAlterationService::getTile(int level, int column) const
	{
		if (level < 0)
		{
			throw std::runtime_error("Cannot retrieve above ground TerrainTile");
		}

		const unsigned xCoord = static_cast<unsigned>(std::abs(column)) - (column == 0 ? 0 : 1);
		if (column < 0)
		{
			return _terrainData.rows[static_cast<unsigned>(level)].leftTiles[xCoord];
		}
		if (column > 0)
		{
			return _terrainData.rows[static_cast<unsigned>(level)].rightTiles[xCoord];
		}

		throw std::runtime_error("Cannot retrieve shaft TerrainTile");
	}

	bool TerrainAlterationService::isTileDugOut(int level, int column) const
	{
		if (doesTileExist(level, column))
		{
			return getTile(level, column).dugOut;
		}
		return false;
	}

	void TerrainAlterationService::initialiseTile(int level, int column)
	{
		if (doesTileExist(level, column))
		{
			return;
		}

		while (static_cast<int>(_terrainData.rows.size()) <= level)
		{
			_terrainData.rows.emplace_back();
		}

		TerrainRow& row = _terrainData.rows[static_cast<unsigned>(level)];
		const int xCoord = std::abs(column) - 1;

		if (column > 0)
		{
			while (static_cast<int>(row.rightTiles.size()) <= xCoord)
			{
				row.rightTiles.emplace_back();
			}
		}

		if (column < 0)
		{
			while (static_cast<int>(row.leftTiles.size()) <= xCoord)
			{
				row.leftTiles.emplace_back();
			}
		}
	}

	bool TerrainAlterationService::doesLevelExist(int level) const
	{
		return _terrainData.shaftLevel >= level;
	}

	bool TerrainAlterationService::canTileBeReached(int level, int column) const
	{
		if (level < 0)
		{
			return true;
		}

		if (!doesLevelExist(level))
		{
			return false;
		}

		bool valid = column == 0;
		for (int x = 1; x <= std::abs(column); ++x)
		{
			const auto sideColumn = column < 0 ? -x : +x;
			if (doesTileExist(level, sideColumn))
			{
				auto tile = getTile(level, sideColumn);
				if (!tile.dugOut)
				{
					if (sideColumn != column || !tile.jobReserved)
					{
						return false;
					}
				}
				valid = true;
			}
			else
			{
				return false;
			}
		}

		return valid;
	}

	bool TerrainAlterationService::isLevelNextShaftLevel(int level) const
	{
		return _terrainData.shaftLevel == level - 1;
	}

}
