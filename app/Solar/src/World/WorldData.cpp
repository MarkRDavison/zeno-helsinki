#include <World/WorldData.hpp>

namespace slr
{

	const StarSystem* WorldData::systemById(long long id) const
	{
		for (const auto& system : systems)
		{
			if (system.id == id)
			{
				return &system;
			}
		}

		return nullptr;
	}

	StarSystem* WorldData::systemById(long long id)
	{
		return const_cast<StarSystem*>(static_cast<const WorldData*>(this)->systemById(id));
	}

	long long getStockpileAmount(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.goodAmount;
			}
		}

		return 0;
	}

}
