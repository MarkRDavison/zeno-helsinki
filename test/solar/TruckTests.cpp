#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace TruckTests
{

	bool truckOnRimwardHelionRoute(const Truck& truck)
	{
		const std::vector<long long> expected{ 5, 3, 2, 1 };
		return truck.motion.route == expected;
	}

TEST_CASE("starter map has trucks on the Rimward-Helion route", "[slr][Truck]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.trucks.size() >= 1);

	int onRoute = 0;
	for (const auto& truck : world.trucks)
	{
		if (truckOnRimwardHelionRoute(truck))
		{
			++onRoute;
		}
	}

	REQUIRE(onRoute >= 1);
}

TEST_CASE("Helion starts with steel need 1000 and Rimward steel need is 0", "[slr][Truck]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	const StarSystem* rimward = world.systemById(5);
	REQUIRE(helion != nullptr);
	REQUIRE(rimward != nullptr);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1000);
	REQUIRE(getNeed(*rimward, SteelGoodId) == 0);
}

TEST_CASE("trucks at a system are those sitting on the node", "[slr][Truck]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(countTrucksAtSystem(world, 5) == 1);
	REQUIRE(countTrucksAtSystem(world, 1) == 0);
	REQUIRE(countTrucksAtSystem(world, 3) == 0);
}

TEST_CASE("after enough steps a truck is in progress on a hop", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	for (int i = 0; i < 4; ++i)
	{
		stepGalaxy(world);
	}

	bool onLane = false;
	for (const auto& truck : world.trucks)
	{
		if (truck.motion.progress > 0.0f && truck.motion.progress < 1.0f)
		{
			onLane = true;
		}
	}

	REQUIRE(onLane);
}

TEST_CASE("truck hop progress this tick is speed over lane length", "[slr][Truck]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "B", .position = { 200.0f, 0.0f } },
	};
	world.hyperlanes.push_back(Hyperlane{ .a = 1, .b = 2 });
	world.trucks.push_back(Truck{
		.id = 1,
		.motion = {
			.route = { 1, 2 },
			.hop = 0,
			.progress = 0.0f,
			.direction = 1,
			.speed = 50.0f },
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel });

	stepGalaxy(world);

	REQUIRE(world.trucks[0].motion.progress == Catch::Approx(0.25f));
	REQUIRE(world.trucks[0].motion.hop == 0);
}

TEST_CASE("truck loads at Rimward only when Helion need remains", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.trucks.push_back(Truck{
		.id = 1,
		.motion = { .route = { 5, 3, 2, 1 }, .hop = 0, .progress = 0.0f, .direction = 1 },
		.cargoGoodId = 0,
		.cargoAmount = 0 });

	stepGalaxy(world);

	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == 50);
	REQUIRE(world.trucks[0].cargoGoodId == SteelGoodId);
	REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_Steel);
}

TEST_CASE("truck dump raises Helion stockpile and lowers need", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.mines.clear();
	world.trucks.push_back(Truck{
		.id = 1,
		.motion = { .route = { 5, 3, 2, 1 }, .hop = 3, .progress = 0.0f, .direction = -1 },
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel });

	stepGalaxy(world);

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(getNeed(*helion, SteelGoodId) == 950);
	REQUIRE(world.trucks[0].cargoAmount == 0);
}

TEST_CASE("empty truck at Rimward does not load when Helion need is 0", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	helion->stocks[0].need = 0;

	world.trucks.push_back(Truck{
		.id = 1,
		.motion = { .route = { 5, 3, 2, 1 }, .hop = 0, .progress = 0.0f, .direction = 1 },
		.cargoGoodId = 0,
		.cargoAmount = 0 });

	stepGalaxy(world);

	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == 100);
	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(world.trucks[0].motion.progress == 0.0f);
	REQUIRE(world.trucks[0].motion.hop == 0);
}

}
}
