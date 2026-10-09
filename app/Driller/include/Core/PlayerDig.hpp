#pragma once

#include <Services/EconomyResourceService.hpp>
#include <Services/TerrainAlterationService.hpp>

namespace drl
{

	inline constexpr long long kPlayerShaftDigCostPerLevel = 100;

	bool tryPlayerDigShaft(
		ITerrainAlterationService& terrain,
		IEconomyResourceService& economy,
		int level);

}
