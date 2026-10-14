#include <Views/GalaxyDrawConstants.hpp>
#include <World/GameCommand.hpp>
#include <World/PickFleet.hpp>
#include <World/PickStarSystem.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

namespace slr
{
namespace FleetOrderTests
{

TEST_CASE("execute move Helion to Kestrel then they arrive", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	REQUIRE_FALSE(world.fleets.empty());
	const long long fleetId = world.fleets.front().id;

	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, 2, CommandSource::Player)));
	REQUIRE(world.fleets.front().motion.route.size() >= 2);
	REQUIRE(world.fleets.front().motion.route.front() == 1);
	REQUIRE(world.fleets.front().motion.route.back() == 2);
	REQUIRE(world.fleets.front().placeId == 0);

	for (int i = 0; i < 64; ++i)
	{
		stepGalaxy(world);
		if (world.fleets.front().systemId == 2 && world.fleets.front().motion.route.empty())
		{
			break;
		}
	}

	REQUIRE(world.fleets.front().systemId == 2);
	REQUIRE(world.fleets.front().motion.route.empty());
}

TEST_CASE("execute Helion to Rimward uses fewest hops 1-2-3-5", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	const long long fleetId = world.fleets.front().id;
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, 5, CommandSource::Player)));
	REQUIRE(world.fleets.front().motion.route == std::vector<long long>{ 1, 2, 3, 5 });
}

TEST_CASE("execute drops already there, no path, and in transit", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	const long long fleetId = world.fleets.front().id;
	REQUIRE_FALSE(execute(world, GameCommand::moveFleetToSystem(fleetId, 1, CommandSource::Player)));

	WorldData isolated;
	isolated.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 9, .name = "B", .position = { 100.0f, 0.0f } },
	};
	isolated.fleets.push_back(Fleet{
		.id = 1,
		.name = "F",
		.systemId = 1,
		.motion = { .speed = 50.0f } });
	REQUIRE_FALSE(execute(isolated, GameCommand::moveFleetToSystem(1, 9, CommandSource::Player)));
	REQUIRE(isolated.fleets.front().systemId == 1);
	REQUIRE(isolated.fleets.front().motion.route.empty());

	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, 2, CommandSource::Player)));
	world.fleets.front().motion.progress = 0.25f;
	REQUIRE_FALSE(execute(world, GameCommand::moveFleetToSystem(fleetId, 3, CommandSource::Player)));
	REQUIRE(world.fleets.front().motion.route.back() == 2);
}

TEST_CASE("fleet marker on a hop is not the from-node", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(world.fleets.front().id, 2, CommandSource::Player)));
	world.fleets.front().motion.progress = 0.5f;

	const auto marker = fleetMarkerPosition(world, world.fleets.front());
	REQUIRE(marker.has_value());
	const StarSystem* helion = world.systemById(1);
	const StarSystem* kestrel = world.systemById(2);
	REQUIRE(helion != nullptr);
	REQUIRE(kestrel != nullptr);
	REQUIRE(*marker == glm::mix(helion->position, kestrel->position, 0.5f));
	REQUIRE(*marker != helion->position + GalaxyFleetOffset);
}

TEST_CASE("pickStarSystem still hits Helion; trucks are not fleets", "[slr][FleetOrder]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	const auto hit = pickStarSystem(world, helion->position, GalaxyNodeSize * 0.5f);
	REQUIRE(hit.has_value());
	REQUIRE(*hit == 1);
	REQUIRE_FALSE(pickFleet(world, helion->position, GalaxyFleetSize * 0.5f).has_value());
}

}
}
