#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace CivilianConsumptionTests
{

TEST_CASE("Helion yard population eats 10 steel and reserves 200", "[slr][CivilianConsumption]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.yards.size() == 1);
	REQUIRE(quantityForGood(world.yards.front().populationConsumePerTick, SteelGoodId) ==
			YardPopulationConsumePerTick_Steel);
	REQUIRE(quantityForGood(world.yards.front().populationReservedPiles, SteelGoodId) ==
			YardPopulationReserved_Steel);
	REQUIRE(quantityForGood(world.yards.front().desiredPiles, SteelGoodId) == 1000);
	REQUIRE(consumerDesiredPile(world, 1, SteelGoodId) == 1200);
	REQUIRE(getNeed(*world.systemById(1), SteelGoodId) == 1200);
}

TEST_CASE("population uses 10 steel when the pile covers it", "[slr][CivilianConsumption]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	for (auto& stock : helion->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 100;
			stock.inbound = 0;
			stock.need = 0;
			stock.deferredNeed = 0;
			stock.deferredNeedHeldOneTick = false;
		}
	}

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 90);
	REQUIRE(helion->stocks.front().deferredNeed == YardPopulationConsumePerTick_Steel);
	REQUIRE(getNeed(*helion, SteelGoodId) == 0);
}

TEST_CASE("population waits when the pile is short of a tick of consumption", "[slr][CivilianConsumption]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	for (auto& stock : helion->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 5;
			stock.inbound = 0;
			stock.need = 0;
			stock.deferredNeed = 0;
			stock.deferredNeedHeldOneTick = false;
		}
	}

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 5);
	REQUIRE(helion->stocks.front().deferredNeed == 0);
}

TEST_CASE("yard consume floor is pile minus population reserved", "[slr][CivilianConsumption]")
{
	WorldData world = makeStarterMap();
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	helion->stocks.front().goodAmount = 250;
	REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 50);

	helion->stocks.front().goodAmount = 150;
	REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 0);
}

}
}
