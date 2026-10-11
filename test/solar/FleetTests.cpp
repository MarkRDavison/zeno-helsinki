#include <Views/GalaxyDrawConstants.hpp>
#include <World/PickFleet.hpp>
#include <World/PickStarSystem.hpp>
#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace FleetTests
{

	const ShipClass* classNamed(const WorldData& world, const char* name)
	{
		for (const auto& shipClass : world.shipClasses)
		{
			if (shipClass.name == name)
			{
				return &shipClass;
			}
		}

		return nullptr;
	}

	long long compositionCount(const Fleet& fleet, long long classId)
	{
		for (const auto& row : fleet.composition)
		{
			if (row.classId == classId)
			{
				return row.count;
			}
		}

		return 0;
	}

TEST_CASE("starter map seeds Nike and Destroyer classes and Home Fleet at Helion", "[slr][Fleet]")
{
	const WorldData world = makeStarterMap();
	const ShipClass* nike = classNamed(world, "Nike");
	const ShipClass* destroyer = classNamed(world, "Destroyer");
	REQUIRE(nike != nullptr);
	REQUIRE(destroyer != nullptr);
	REQUIRE(nike->id != 0);
	REQUIRE(destroyer->id != 0);
	REQUIRE(nike->id != destroyer->id);

	REQUIRE(world.fleets.size() == 1);
	const Fleet& fleet = world.fleets.front();
	REQUIRE(fleet.name == "Home Fleet");
	REQUIRE(fleet.systemId == 1);
	REQUIRE(compositionCount(fleet, nike->id) == 8);
	REQUIRE(compositionCount(fleet, destroyer->id) == 12);
}

TEST_CASE("pick hits the fleet offset marker and misses Helion node centre", "[slr][Fleet]")
{
	const WorldData world = makeStarterMap();
	REQUIRE_FALSE(world.fleets.empty());
	const auto marker = fleetMarkerPosition(world, world.fleets.front(), GalaxyFleetOffset);
	REQUIRE(marker.has_value());

	const float fleetRadius = GalaxyFleetSize * 0.5f;
	const float systemRadius = GalaxyNodeSize * 0.5f;
	REQUIRE(glm::length(GalaxyFleetOffset) > fleetRadius + systemRadius);

	const auto fleetHit = pickFleet(world, *marker, fleetRadius, GalaxyFleetOffset);
	REQUIRE(fleetHit.has_value());
	REQUIRE(*fleetHit == world.fleets.front().id);

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE_FALSE(pickFleet(world, helion->position, fleetRadius, GalaxyFleetOffset).has_value());
	REQUIRE_FALSE(pickStarSystem(world, *marker, systemRadius).has_value());
}

TEST_CASE("pickStarSystem still hits Helion at its position", "[slr][Fleet]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	const auto hit = pickStarSystem(world, helion->position, GalaxyNodeSize * 0.5f);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == 1);
}

TEST_CASE("trucks are not pickable as fleets", "[slr][Fleet]")
{
	const WorldData world = makeStarterMap();
	REQUIRE_FALSE(world.trucks.empty());
	const Truck& truck = world.trucks.front();
	REQUIRE(truck.route.size() >= 2);
	const StarSystem* from = world.systemById(truck.route[static_cast<size_t>(truck.hop)]);
	const int toIndex = truck.hop + truck.direction;
	REQUIRE(from != nullptr);
	REQUIRE(toIndex >= 0);
	REQUIRE(toIndex < static_cast<int>(truck.route.size()));
	const StarSystem* to = world.systemById(truck.route[static_cast<size_t>(toIndex)]);
	REQUIRE(to != nullptr);

	const glm::vec2 truckPos = glm::mix(from->position, to->position, truck.progress);
	REQUIRE_FALSE(pickFleet(world, truckPos, GalaxyFleetSize * 0.5f, GalaxyFleetOffset).has_value());
}

}
}
