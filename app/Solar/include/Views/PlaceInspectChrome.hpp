#pragma once

#include <World/GameCommand.hpp>
#include <World/WorldData.hpp>
#include <vector>

namespace slr
{

	void drawYardPlaceInspector(
		const WorldData& world,
		long long systemId,
		std::vector<GameCommand>& pendingCommands);
	void drawMinePlaceInspector(const WorldData& world, long long systemId);
	void drawFactoryPlaceInspector(const WorldData& world, long long systemId);

}
