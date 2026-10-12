#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace JobTests
{

TEST_CASE("starter Helion haul job is uncovered steel amount, not a truck", "[slr][Job]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(uncoveredNeed(*helion, SteelGoodId) == 950);

	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	REQUIRE(job->destinationSystemId == 1);
	REQUIRE(job->goodId == SteelGoodId);
	REQUIRE(job->amount == uncoveredNeed(*helion, SteelGoodId));
	REQUIRE(job->amount == 950);
	REQUIRE(world.haulJobById(job->id) == job);
}

TEST_CASE("Rimward has no haul job", "[slr][Job]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(haulJobFor(world, 5, SteelGoodId) == nullptr);
}

TEST_CASE("uncovered need below a truckload removes the haul job", "[slr][Job]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.need = TruckCapacity_Steel - 1 });
	syncHaulJobsFromUncoveredNeed(world);
	REQUIRE(haulJobFor(world, 1, SteelGoodId) == nullptr);
}

TEST_CASE("a convoy of 19 steel trucks can carry Helion's 950 in one go; two cannot", "[slr][Job]")
{
	REQUIRE(convoyCarriesAmountInOneGo(19, 950, SteelGoodId));
	REQUIRE_FALSE(convoyCarriesAmountInOneGo(2, 950, SteelGoodId));
}

TEST_CASE("150 steel needs three trucks in one go", "[slr][Job]")
{
	REQUIRE(truckCountToCarryAmountInOneGo(150, SteelGoodId) == 3);
}

}
}
