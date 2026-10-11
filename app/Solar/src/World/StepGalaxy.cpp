#include <World/StepGalaxy.hpp>

namespace slr
{
namespace
{

	void addToStockpile(StarSystem& system, long long goodId, long long amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				stock.goodAmount += amount;
				return;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = goodId, .goodAmount = amount });
	}

	void applyExtraction(WorldData& world)
	{
		for (const auto& mine : world.mines)
		{
			StarSystem* system = world.systemById(mine.systemId);
			if (system == nullptr)
			{
				continue;
			}

			addToStockpile(*system, mine.goodId, mine.outputPerTick);
		}
	}

}

	void stepGalaxy(WorldData& world)
	{
		applyExtraction(world);
		++world.tick;
	}

}
