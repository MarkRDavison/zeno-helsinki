#include <Views/GalaxyDrawConstants.hpp>
#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/PickFleet.hpp>
#include <World/PickStarSystem.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/SystemPlaces.hpp>
#include <PutAtJump.hpp>
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
	putFleetAtJumpToward(world, fleetId, KestrelSystemId);

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
	const SystemPlace* kestrelHelionJump =
		jumpPlaceForLaneNeighbor(world, KestrelSystemId, HelionSystemId);
	REQUIRE(kestrelHelionJump != nullptr);
	REQUIRE(world.fleets.front().placeId == kestrelHelionJump->id);
	REQUIRE(world.fleets.front().placeId != KestrelFactoryPlaceId);
}

TEST_CASE("execute Helion to Rimward uses fewest hops 1-2-3-5", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	const long long fleetId = world.fleets.front().id;
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, 5, CommandSource::Player)));
	REQUIRE(world.fleets.front().inSystem.pendingGalaxyRoute == std::vector<long long>{ 1, 2, 3, 5 });
	stepUntilFleetHasGalaxyRoute(world, fleetId);
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

	putFleetAtJumpToward(world, fleetId, KestrelSystemId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, 2, CommandSource::Player)));
	world.fleets.front().motion.progress = 0.25f;
	REQUIRE_FALSE(execute(world, GameCommand::moveFleetToSystem(fleetId, 3, CommandSource::Player)));
	REQUIRE(world.fleets.front().motion.route.back() == 2);
}

TEST_CASE("fleet marker on a hop is not the from-node", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	putFleetAtJumpToward(world, world.fleets.front().id, KestrelSystemId);
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

TEST_CASE("MoveFleetToPlace crawls Home Fleet from the Kestrel jump to the yard", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	putFleetAtJumpToward(world, 1, KestrelSystemId);
	REQUIRE(home->placeId == HelionKestrelJumpPlaceId);
	REQUIRE(execute(world, GameCommand::moveFleetToPlace(1, HelionYardPlaceId, CommandSource::Player)));
	REQUIRE(home->placeId == 0);
	REQUIRE(inSystemMotionIsCrawling(home->inSystem));

	for (int i = 0; i < 64; ++i)
	{
		stepGalaxy(world);
		if (home->placeId == HelionYardPlaceId)
		{
			break;
		}
	}

	REQUIRE(home->placeId == HelionYardPlaceId);
	REQUIRE_FALSE(inSystemMotionIsCrawling(home->inSystem));
}

TEST_CASE("MoveFleetToPlace on the jump they already sit on holds", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	putFleetAtJumpToward(world, 1, KestrelSystemId);
	REQUIRE(execute(
		world,
		GameCommand::moveFleetToPlace(1, HelionKestrelJumpPlaceId, CommandSource::Player)));
	REQUIRE(world.fleets.front().placeId == HelionKestrelJumpPlaceId);
	REQUIRE_FALSE(inSystemMotionIsCrawling(world.fleets.front().inSystem));
}

TEST_CASE("MoveFleetToSystem from the Helion yard crawls to the Kestrel jump then FTL", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	const long long fleetId = world.fleets.front().id;
	REQUIRE(world.fleets.front().placeId == HelionYardPlaceId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, KestrelSystemId, CommandSource::Player)));
	REQUIRE(inSystemMotionIsCrawling(world.fleets.front().inSystem));
	REQUIRE(world.fleets.front().motion.route.size() < 2);
	stepUntilFleetHasGalaxyRoute(world, fleetId);
	REQUIRE(world.fleets.front().motion.route.front() == HelionSystemId);
	REQUIRE(world.fleets.front().motion.route.back() == KestrelSystemId);
}

TEST_CASE("Kestrel Fleet to Rimward auto-crawls intermediates and stops at Rimward entry jump", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	Fleet* kestrel = world.fleetById(3);
	REQUIRE(kestrel != nullptr);
	REQUIRE(kestrel->placeId == KestrelFactoryPlaceId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(3, RimwardSystemId, CommandSource::Player)));
	REQUIRE(inSystemMotionIsCrawling(kestrel->inSystem));

	const SystemPlace* junctionKestrelJump =
		jumpPlaceForLaneNeighbor(world, JunctionSystemId, KestrelSystemId);
	const SystemPlace* junctionRimwardJump =
		jumpPlaceForLaneNeighbor(world, JunctionSystemId, RimwardSystemId);
	REQUIRE(junctionKestrelJump != nullptr);
	REQUIRE(junctionRimwardJump != nullptr);
	bool crawledJunction = false;
	for (int i = 0; i < 256; ++i)
	{
		stepGalaxy(world);
		if (kestrel->systemId == JunctionSystemId && inSystemMotionIsCrawling(kestrel->inSystem) &&
			kestrel->inSystem.fromPlaceId == junctionKestrelJump->id &&
			kestrel->inSystem.toPlaceId == junctionRimwardJump->id)
		{
			crawledJunction = true;
		}

		if (kestrel->systemId == RimwardSystemId && kestrel->motion.route.empty() &&
			!inSystemMotionIsCrawling(kestrel->inSystem))
		{
			break;
		}
	}

	REQUIRE(crawledJunction);

	REQUIRE(kestrel->systemId == RimwardSystemId);
	REQUIRE(kestrel->motion.route.empty());
	const SystemPlace* rimwardJunctionJump =
		jumpPlaceForLaneNeighbor(world, RimwardSystemId, JunctionSystemId);
	REQUIRE(rimwardJunctionJump != nullptr);
	REQUIRE(kestrel->placeId == rimwardJunctionJump->id);
	REQUIRE(kestrel->placeId != RimwardMinePlaceId);
}

TEST_CASE("Helion to Junction crawls Kestrel from the Helion jump to the Junction jump", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	putFleetAtJumpToward(world, 1, KestrelSystemId);
	REQUIRE(home->placeId == HelionKestrelJumpPlaceId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(1, JunctionSystemId, CommandSource::Player)));

	const SystemPlace* kestrelHelionJump =
		jumpPlaceForLaneNeighbor(world, KestrelSystemId, HelionSystemId);
	const SystemPlace* kestrelJunctionJump =
		jumpPlaceForLaneNeighbor(world, KestrelSystemId, JunctionSystemId);
	const SystemPlace* junctionKestrelJump =
		jumpPlaceForLaneNeighbor(world, JunctionSystemId, KestrelSystemId);
	REQUIRE(kestrelHelionJump != nullptr);
	REQUIRE(kestrelJunctionJump != nullptr);
	REQUIRE(junctionKestrelJump != nullptr);

	bool crawledKestrel = false;
	for (int i = 0; i < 256; ++i)
	{
		stepGalaxy(world);
		if (home->systemId == KestrelSystemId && inSystemMotionIsCrawling(home->inSystem) &&
			home->inSystem.fromPlaceId == kestrelHelionJump->id &&
			home->inSystem.toPlaceId == kestrelJunctionJump->id)
		{
			crawledKestrel = true;
			REQUIRE(home->motion.route.size() < 2);
		}

		if (home->systemId == JunctionSystemId && home->motion.route.empty() &&
			!inSystemMotionIsCrawling(home->inSystem))
		{
			break;
		}
	}

	REQUIRE(crawledKestrel);
	REQUIRE(home->systemId == JunctionSystemId);
	REQUIRE(home->placeId == junctionKestrelJump->id);
	REQUIRE(home->placeId != JunctionFactoryPlaceId);
}

TEST_CASE("Kestrel arrival then return sits at Helion Kestrel jump not the yard", "[slr][FleetOrder]")
{
	WorldData world = makeStarterMap();
	const long long fleetId = world.fleets.front().id;
	putFleetAtJumpToward(world, fleetId, KestrelSystemId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, KestrelSystemId, CommandSource::Player)));
	for (int i = 0; i < 64; ++i)
	{
		stepGalaxy(world);
		if (world.fleets.front().systemId == KestrelSystemId && world.fleets.front().motion.route.empty())
		{
			break;
		}
	}

	REQUIRE(world.fleets.front().systemId == KestrelSystemId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, HelionSystemId, CommandSource::Player)));
	for (int i = 0; i < 64; ++i)
	{
		stepGalaxy(world);
		if (world.fleets.front().systemId == HelionSystemId && world.fleets.front().motion.route.empty())
		{
			break;
		}
	}

	REQUIRE(world.fleets.front().systemId == HelionSystemId);
	REQUIRE(world.fleets.front().placeId == HelionKestrelJumpPlaceId);
	REQUIRE(world.fleets.front().placeId != HelionYardPlaceId);
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
