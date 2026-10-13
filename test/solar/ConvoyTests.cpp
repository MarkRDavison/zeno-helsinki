#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/TruckCargo.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace ConvoyTests
{

TEST_CASE("starter in-flight truck is a size-1 departed convoy", "[slr][Convoy]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(
		world.trucks.size() ==
		StarterCivilianTruckCount + StarterMilitaryTruckCount + StarterTenderCount);
	const Truck* flying = world.truckById(StarterCivilianTruckCount);
	REQUIRE(flying != nullptr);
	REQUIRE(flying->convoyId != 0);
	REQUIRE(world.trucks[0].convoyId == 0);
	REQUIRE(world.trucks[1].convoyId == 0);
	const Convoy* convoy = world.convoyById(flying->convoyId);
	REQUIRE(convoy != nullptr);
	REQUIRE(countTrucksInConvoy(world, convoy->id) == 1);
	REQUIRE(countTrucksInConvoyPresent(world, *convoy) == 1);
	REQUIRE(convoyIsUnderway(*convoy));
}

TEST_CASE("two Rimward empties join one new convoy; size 1 still exists", "[slr][Convoy]")
{
	WorldData world = makeStarterMap();
	const long long flyingConvoyId = world.truckById(StarterCivilianTruckCount)->convoyId;

	stepGalaxy(world);

	REQUIRE(world.trucks[0].convoyId != 0);
	REQUIRE(world.trucks[0].convoyId == world.trucks[1].convoyId);
	REQUIRE(world.trucks[0].convoyId != flyingConvoyId);
	REQUIRE(countTrucksInConvoy(world, world.trucks[0].convoyId) == 2);
	REQUIRE(countTrucksInConvoy(world, flyingConvoyId) == 1);
	REQUIRE(convoyIsUnderway(*world.convoyById(world.trucks[0].convoyId)));
	REQUIRE(convoyIsUnderway(*world.convoyById(flyingConvoyId)));
}

TEST_CASE("loaded truck waits at supply for a straggler on the same convoy", "[slr][Convoy]")
{
	WorldData world = makeStarterMap();
	world.trucks.erase(world.trucks.begin() + 2, world.trucks.end());
	world.convoys.clear();
	world.trucks[1].motion.route = { 3 };
	world.trucks[1].motion.hop = 0;
	world.trucks[1].motion.progress = 0.0f;

	stepGalaxy(world);

	REQUIRE(world.trucks[0].convoyId != 0);
	REQUIRE(world.trucks[0].convoyId == world.trucks[1].convoyId);
	REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_IronOre);
	REQUIRE(isTruckAtSystem(world.trucks[0], 5));
	REQUIRE_FALSE(convoyIsUnderway(*world.convoyById(world.trucks[0].convoyId)));
	REQUIRE(world.trucks[0].motion.route == std::vector<long long>{ 5 });
	REQUIRE(world.trucks[1].cargoAmount == 0);
	REQUIRE_FALSE(isTruckAtSystem(world.trucks[1], 5));
	REQUIRE(countTrucksInConvoyPresent(world, *world.convoyById(world.trucks[0].convoyId)) == 1);
	REQUIRE(countTrucksInConvoy(world, world.trucks[0].convoyId) == 2);

	const long long convoyId = world.trucks[0].convoyId;
	while (!convoyIsUnderway(*world.convoyById(convoyId)))
	{
		REQUIRE(world.tick < 40);
		REQUIRE(isTruckAtSystem(world.trucks[0], 5));
		stepGalaxy(world);
	}
	REQUIRE(world.trucks[0].motion.route == shortestLanePath(world, 5, 3));
	REQUIRE(world.trucks[1].motion.route == world.trucks[0].motion.route);
}

TEST_CASE("assembled convoy hops at the slowest member speed", "[slr][Convoy]")
{
	WorldData world = makeStarterMap();
	world.trucks.erase(world.trucks.begin() + 2, world.trucks.end());
	world.convoys.clear();
	world.trucks[0].motion.speed = 20.0f;
	world.trucks[1].motion.speed = 25.0f;

	stepGalaxy(world);

	const Convoy* convoy = world.convoyById(world.trucks[0].convoyId);
	REQUIRE(convoy != nullptr);
	REQUIRE(convoyIsUnderway(*convoy));
	long long fromId = 0;
	long long toId = 0;
	REQUIRE(hopEndpoints(world.trucks[0].motion, fromId, toId));
	const StarSystem* from = world.systemById(fromId);
	const StarSystem* to = world.systemById(toId);
	REQUIRE(from != nullptr);
	REQUIRE(to != nullptr);
	const float length = glm::length(to->position - from->position);
	const float expected = 20.0f / length;
	REQUIRE(world.trucks[0].motion.progress == Catch::Approx(expected));
	REQUIRE(world.trucks[1].motion.progress == Catch::Approx(expected));
	REQUIRE(world.trucks[0].motion.hop == world.trucks[1].motion.hop);
	REQUIRE(world.trucks[0].motion.speed == Catch::Approx(20.0f));
	REQUIRE(world.trucks[1].motion.speed == Catch::Approx(25.0f));
}

TEST_CASE("destroy a waiting empty lets the loaded truck depart", "[slr][Convoy]")
{
	WorldData world = makeStarterMap();
	world.trucks.erase(world.trucks.begin() + 2, world.trucks.end());
	world.convoys.clear();
	world.trucks[1].motion.route = { 3 };
	world.trucks[1].motion.hop = 0;
	world.trucks[1].motion.progress = 0.0f;

	stepGalaxy(world);

	REQUIRE_FALSE(convoyIsUnderway(*world.convoyById(world.trucks[0].convoyId)));
	REQUIRE(destroyTruckAndReduceInbound(world, world.trucks[1].id));

	stepGalaxy(world);

	REQUIRE(world.trucks.size() == 1);
	REQUIRE(convoyIsUnderway(*world.convoyById(world.trucks[0].convoyId)));
	REQUIRE(world.trucks[0].motion.route == shortestLanePath(world, 5, 3));
}

}
}
