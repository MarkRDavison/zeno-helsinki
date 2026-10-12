#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace NeedTests
{

TEST_CASE("empty consumer with desired 1000 posts need 1000 at map create", "[slr][Need]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);
	REQUIRE(consumerDesiredPile(world, 1, SteelGoodId) == 1000);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1000);
	REQUIRE(consumerDesiredPile(world, 3, IronOreGoodId) == 1000);
	REQUIRE(getNeed(*world.systemById(3), IronOreGoodId) == 1000);
}

TEST_CASE("full desired pile posts no need", "[slr][Need]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.goodAmount = 1000 });
	world.yards.push_back(Yard{
		.systemId = 1,
		.desiredPiles = { GoodQuantity{ .goodId = SteelGoodId, .amount = 1000 } } });
	postNeedFromConsumerShortfall(world, world.systems.front(), SteelGoodId);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 0);
}

TEST_CASE("using 500 from 1000 does not post need until the second galaxy tick", "[slr][Need]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.goodAmount = 1000 });

	REQUIRE(useFromStockpile(world, 1, SteelGoodId, 500));
	REQUIRE(getStockpileAmount(world.systems.front(), SteelGoodId) == 500);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 0);
	REQUIRE(getInbound(world.systems.front(), SteelGoodId) == 0);

	stepGalaxy(world);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 0);

	stepGalaxy(world);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 500);
	REQUIRE(getStockpileAmount(world.systems.front(), SteelGoodId) == 500);
}

TEST_CASE("use below one truckload does not post until deferred need reaches a truckload", "[slr][Need]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.goodAmount = 1000 });

	REQUIRE(useFromStockpile(world, 1, SteelGoodId, 1));
	stepGalaxy(world);
	stepGalaxy(world);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 0);

	REQUIRE(useFromStockpile(world, 1, SteelGoodId, TruckCapacity_Steel - 1));
	stepGalaxy(world);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 0);
	stepGalaxy(world);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == TruckCapacity_Steel);
}

TEST_CASE("use more than on hand does not change stockpile or need", "[slr][Need]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.goodAmount = 100,
		.need = 0 });

	REQUIRE_FALSE(useFromStockpile(world, 1, SteelGoodId, 101));
	REQUIRE(getStockpileAmount(world.systems.front(), SteelGoodId) == 100);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 0);
	REQUIRE(world.systems.front().stocks.front().deferredNeed == 0);
}

TEST_CASE("arrival still lowers need", "[slr][Need]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	helion->stocks[0].need = 1000;
	helion->stocks[0].inbound = TruckCapacity_Steel;

	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = {
			.route = { 1 },
			.hop = 0,
			.progress = 0.0f,
			.direction = 1,
			.speed = 25.0f },
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel,
		.haulJobId = job->id });

	stepGalaxy(world);

	REQUIRE(getInbound(*helion, SteelGoodId) == 0);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1000 - TruckCapacity_Steel);
}

}
}
