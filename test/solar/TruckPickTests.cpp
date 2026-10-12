#include <Views/GalaxyDrawConstants.hpp>
#include <World/LaneMotion.hpp>
#include <World/PickFleet.hpp>
#include <World/PickStarSystem.hpp>
#include <World/PickTruck.hpp>
#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace TruckPickTests
{

TEST_CASE("pick hits the in-flight starter truck", "[slr][Truck][Pick]")
{
	const WorldData world = makeStarterMap();
	const Truck* flying = world.truckById(StarterCivilianTruckCount);
	REQUIRE(flying != nullptr);
	const auto marker = truckMarkerPosition(world, *flying);
	REQUIRE(marker.has_value());
	const auto hit = pickTruck(world, *marker, GalaxyTruckSize * 0.5f);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == flying->id);
	REQUIRE_FALSE(pickFleet(world, *marker, GalaxyFleetSize * 0.5f).has_value());
}

TEST_CASE("parked truck marker misses the system node", "[slr][Truck][Pick]")
{
	const WorldData world = makeStarterMap();
	const Truck* parked = world.truckById(1);
	REQUIRE(parked != nullptr);
	const auto marker = truckMarkerPosition(world, *parked);
	REQUIRE(marker.has_value());
	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);

	const float truckRadius = GalaxyTruckSize * 0.5f;
	const float systemRadius = GalaxyNodeSize * 0.5f;
	REQUIRE(glm::length(GalaxyTruckOffset) > truckRadius + systemRadius);

	const auto truckHit = pickTruck(world, *marker, truckRadius);
	REQUIRE(truckHit.has_value());
	REQUIRE(*truckHit == parked->id);
	REQUIRE_FALSE(pickStarSystem(world, *marker, systemRadius).has_value());
	REQUIRE_FALSE(pickTruck(world, rimward->position, truckRadius).has_value());
}

TEST_CASE("stacked convoy members pick the smaller truck id", "[slr][Truck][Pick]")
{
	WorldData world = makeStarterMap();
	const Truck* flying = world.truckById(StarterCivilianTruckCount);
	REQUIRE(flying != nullptr);
	world.trucks[0].motion = flying->motion;
	world.trucks[0].convoyId = flying->convoyId;
	const auto marker = truckMarkerPosition(world, *flying);
	REQUIRE(marker.has_value());
	const auto hit = pickTruck(world, *marker, GalaxyTruckSize * 0.5f);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == world.trucks[0].id);
}

}
}
