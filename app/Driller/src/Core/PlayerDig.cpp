#include <Core/PlayerDig.hpp>

namespace drl
{

	bool tryPlayerDigShaft(
		ITerrainAlterationService& terrain,
		IEconomyResourceService& economy,
		int level)
	{
		if (!terrain.isLevelNextShaftLevel(level))
		{
			return false;
		}

		const long long cost = kPlayerShaftDigCostPerLevel * static_cast<long long>(level);
		if (!economy.canAfford(ResourceMoney, cost))
		{
			return false;
		}

		if (!economy.pay(ResourceMoney, cost))
		{
			return false;
		}

		return terrain.digShaft(level);
	}

}
