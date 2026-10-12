#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace FactoryTests
{

TEST_CASE("starter Rimward mines ore; Junction mill wants ore; Helion wants steel", "[slr][Factory]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.mines.size() == 1);
	REQUIRE(mineOutputPerTick(world.mines[0], IronOreGoodId) == 100);
	REQUIRE(mineOutputPerTick(world.mines[0], SteelGoodId) == 0);
	REQUIRE(world.factories.size() == 1);
	REQUIRE(world.factories[0].systemId == 3);
	REQUIRE(quantityForGood(world.factories[0].desiredPiles, IronOreGoodId) == 1000);
	REQUIRE(consumerDesiredPile(world, 3, IronOreGoodId) == 1000);
	REQUIRE(consumerDesiredPile(world, 1, SteelGoodId) == 1200);
	REQUIRE(getNeed(*world.systemById(3), IronOreGoodId) == 1000);
	REQUIRE(getNeed(*world.systemById(1), SteelGoodId) == 1200);
	REQUIRE(haulJobFor(world, 3, IronOreGoodId) != nullptr);
	REQUIRE(haulJobFor(world, 3, IronOreGoodId)->amount == 1000);
}

TEST_CASE("mill eats a truckload of ore and makes a truckload of steel", "[slr][Factory]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* junction = world.systemById(3);
	REQUIRE(junction != nullptr);
	junction->stocks[0].goodAmount = 50;
	const long long oreNeedBefore = getNeed(*junction, IronOreGoodId);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*junction, IronOreGoodId) == 0);
	REQUIRE(getStockpileAmount(*junction, SteelGoodId) == 50);
	REQUIRE(getNeed(*junction, IronOreGoodId) == oreNeedBefore);

	stepGalaxy(world);
	REQUIRE(getNeed(*junction, IronOreGoodId) == oreNeedBefore);
	stepGalaxy(world);
	REQUIRE(getNeed(*junction, IronOreGoodId) == oreNeedBefore + 50);
}

TEST_CASE("mill does nothing when ore is short of a truckload", "[slr][Factory]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* junction = world.systemById(3);
	REQUIRE(junction != nullptr);
	junction->stocks[0].goodAmount = 49;
	junction->stocks[1].goodAmount = 10;

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*junction, IronOreGoodId) == 49);
	REQUIRE(getStockpileAmount(*junction, SteelGoodId) == 10);
}

TEST_CASE("mill does nothing when steel is at the output cap", "[slr][Factory]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* junction = world.systemById(3);
	REQUIRE(junction != nullptr);
	junction->stocks[0].goodAmount = 50;
	junction->stocks[1].goodAmount = FactoryMaxStockpile_Steel;

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*junction, IronOreGoodId) == 50);
	REQUIRE(getStockpileAmount(*junction, SteelGoodId) == FactoryMaxStockpile_Steel);
}

TEST_CASE("empty truck at Junction with mill steel takes Helion job along Junction to Helion", "[slr][Factory]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* junction = world.systemById(3);
	REQUIRE(junction != nullptr);
	junction->stocks[1].goodAmount = TruckCapacity_Steel;
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 3 }, .speed = 25.0f } });

	stepGalaxy(world);

	REQUIRE(world.trucks[0].haulJobId == haulJobFor(world, 1, SteelGoodId)->id);
	REQUIRE(world.trucks[0].cargoGoodId == SteelGoodId);
	REQUIRE(world.trucks[0].motion.route == shortestLanePath(world, 3, 1));
}

}
}
