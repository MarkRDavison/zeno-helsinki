#pragma once

#include <SceneCatalog.hpp>
#include <cmath>
#include <optional>
#include <vector>

namespace tower
{
	struct PathBlockFootprint
	{
		int x = 0;
		int z = 0;
		int sizeX = 1;
		int sizeZ = 1;
	};

	inline bool footprintContains(const PathBlockFootprint& footprint, int tx, int tz)
	{
		return tx >= footprint.x && tx < footprint.x + footprint.sizeX
			&& tz >= footprint.z && tz < footprint.z + footprint.sizeZ;
	}

	inline std::optional<int> firstBlockedIndex(
		const std::vector<TileCoord>& path,
		int fromIndex,
		const PathBlockFootprint& footprint)
	{
		if (fromIndex < 0)
		{
			return std::nullopt;
		}

		for (int i = fromIndex; i < static_cast<int>(path.size()); ++i)
		{
			const auto& tile = path[static_cast<std::size_t>(i)];
			if (footprintContains(footprint, tile.x, tile.z))
			{
				return i;
			}
		}

		return std::nullopt;
	}

	inline std::optional<TileCoord> firstBlockedTile(
		const std::vector<TileCoord>& path,
		int fromIndex,
		const PathBlockFootprint& footprint)
	{
		const auto index = firstBlockedIndex(path, fromIndex, footprint);
		if (!index.has_value())
		{
			return std::nullopt;
		}

		return path[static_cast<std::size_t>(*index)];
	}

	inline std::optional<int> stallApproachIndex(
		const std::vector<TileCoord>& path,
		int fromIndex,
		const PathBlockFootprint& footprint)
	{
		const auto blocked = firstBlockedIndex(path, fromIndex, footprint);
		if (!blocked.has_value())
		{
			return std::nullopt;
		}

		if (*blocked <= 0)
		{
			return 0;
		}

		return *blocked - 1;
	}

	inline float xzDistance(const glm::vec3& a, const glm::vec3& b)
	{
		const float dx = a.x - b.x;
		const float dz = a.z - b.z;
		return std::sqrt(dx * dx + dz * dz);
	}

	inline bool shouldStall(const glm::vec3& creepPos, const glm::vec3& blockedTileCenter, float range)
	{
		return xzDistance(creepPos, blockedTileCenter) <= range;
	}
}
