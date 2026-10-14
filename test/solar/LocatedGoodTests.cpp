#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/SystemPlaces.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace LocatedGoodTests
{

void clearAvonSteel(WorldData& world)
{
	StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	for (auto& stock : avon->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 0.f;
			return;
		}
	}
}

TEST_CASE("starter located steel is the flying truckload; ore is 0", "[slr][LocatedGood]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(locatedGoodAmount(world, SteelGoodId) == TruckCapacity_Steel + AvonStarterSteelStockpile);
	REQUIRE(locatedGoodAmount(world, IronOreGoodId) == 0);
}

TEST_CASE("loading ore moves it from Rimward onto the truck without changing the located total", "[slr][LocatedGood]")
{
	WorldData world = makeStarterMap();
	clearAvonSteel(world);
	world.mines.clear();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	rimward->stocks[0].goodAmount = TruckCapacity_IronOre;

	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.placeId = RimwardMinePlaceId,
		.classId = HelionCivilianTruckClassId });

	const long long locatedBefore = locatedGoodAmount(world, IronOreGoodId);
	REQUIRE(locatedBefore == TruckCapacity_IronOre);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*rimward, IronOreGoodId) == 0);
	REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_IronOre);
	REQUIRE(locatedGoodAmount(world, IronOreGoodId) == locatedBefore);
}

TEST_CASE("dumping steel moves it from the truck into Helion without changing the located total", "[slr][LocatedGood]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.mines.clear();
	world.factories.clear();
	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	world.yards.clear();
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.placeId = HelionYardPlaceId,
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel,
		.haulJobId = job->id });

	const GoodAmount locatedBefore = locatedGoodAmount(world, SteelGoodId);
	REQUIRE(locatedBefore == TruckCapacity_Steel + AvonStarterSteelStockpile);

	stepGalaxy(world);

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);
	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(locatedGoodAmount(world, SteelGoodId) == locatedBefore);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(locatedGoodAmount(world, SteelGoodId) == locatedBefore);
}

TEST_CASE("civilian consumption lowers located steel", "[slr][LocatedGood]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	helion->stocks.front().goodAmount = 100;
	helion->stocks.front().inbound = 0;
	StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	for (auto& stock : avon->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 0.f;
			break;
		}
	}

	REQUIRE(locatedGoodAmount(world, SteelGoodId) == 100);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 90);
	REQUIRE(locatedGoodAmount(world, SteelGoodId) == 90);
}

TEST_CASE("inbound is not part of located amount", "[slr][LocatedGood]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.goodAmount = 0,
		.inbound = 400 });

	REQUIRE(locatedGoodAmount(world, SteelGoodId) == 0);
}

}
}
