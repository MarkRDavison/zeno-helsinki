#include <Views/GalaxyDrawConstants.hpp>
#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/PickTruck.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>

namespace slr
{
namespace EscortTests
{

	void parkFleetAt(WorldData& world, long long fleetId, long long systemId)
	{
		Fleet* fleet = world.fleetById(fleetId);
		REQUIRE(fleet != nullptr);
		fleet->systemId = systemId;
		fleet->motion.route.clear();
		fleet->motion.hop = 0;
		fleet->motion.progress = 0.0f;
	}

	void keepFirstEmptyTrucks(WorldData& world, std::size_t count)
	{
		world.trucks.erase(world.trucks.begin() + static_cast<std::ptrdiff_t>(count), world.trucks.end());
		world.convoys.clear();
		for (auto& truck : world.trucks)
		{
			truck.haulJobId = 0;
			truck.convoyId = 0;
			truck.cargoGoodId = 0;
			truck.cargoAmount = 0;
			parkTruckAt(truck, 5);
		}
	}

TEST_CASE("escort stance joins empties up to N at the fleet", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 2, 0, 0, CommandSource::Player)));
	const Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	REQUIRE(home->escortConvoyId != 0);
	const Convoy* standing = world.convoyById(home->escortConvoyId);
	REQUIRE(standing != nullptr);
	REQUIRE(convoyIsStanding(*standing));
	REQUIRE(standing->desiredCivilianTruckCount == 2);
	REQUIRE(standing->desiredMilitaryTruckCount == 0);

	stepGalaxy(world);

	const long long standingId = world.fleetById(1)->escortConvoyId;
	REQUIRE(countTrucksInConvoy(world, standingId) == 2);
	REQUIRE(world.trucks[0].convoyId == standingId);
	REQUIRE(world.trucks[1].convoyId == standingId);
}

TEST_CASE("standing group does not take a haul job until N are at the rally", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 2);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 3, 0, 0, CommandSource::Player)));

	stepGalaxy(world);

	const Fleet* home = world.fleetById(1);
	const Convoy* standing = world.convoyById(home->escortConvoyId);
	REQUIRE(standing != nullptr);
	REQUIRE(countTrucksInConvoy(world, standing->id) == 2);
	REQUIRE(standing->haulJobId == 0);
	REQUIRE(world.trucks[0].haulJobId == 0);
	REQUIRE(world.trucks[1].haulJobId == 0);
}

TEST_CASE("dump does not dissolve a standing convoy", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 1);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 0, 0, CommandSource::Player)));

	const long long standingId = world.fleetById(1)->escortConvoyId;
	while (world.trucks[0].cargoAmount == 0 || world.trucks[0].haulJobId == 0)
	{
		REQUIRE(world.tick < 40);
		stepGalaxy(world);
	}

	const long long jobId = world.trucks[0].haulJobId;
	const HaulJob* job = world.haulJobById(jobId);
	REQUIRE(job != nullptr);
	const long long dest = job->destinationSystemId;
	while (world.trucks[0].haulJobId != 0)
	{
		REQUIRE(world.tick < 80);
		stepGalaxy(world);
	}

	REQUIRE(world.trucks[0].convoyId == standingId);
	REQUIRE(world.convoyById(standingId) != nullptr);
	REQUIRE(convoyIsStanding(*world.convoyById(standingId)));
	REQUIRE(isTruckAtSystem(world.trucks[0], dest));
}

TEST_CASE("after dump the standing group can take another job", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 1);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 0, 0, CommandSource::Player)));

	while (world.trucks[0].haulJobId == 0)
	{
		REQUIRE(world.tick < 40);
		stepGalaxy(world);
	}

	while (world.trucks[0].haulJobId != 0)
	{
		REQUIRE(world.tick < 80);
		stepGalaxy(world);
	}

	while (world.trucks[0].haulJobId == 0)
	{
		REQUIRE(world.tick < 120);
		stepGalaxy(world);
	}

	REQUIRE(world.trucks[0].convoyId == world.fleetById(1)->escortConvoyId);
	REQUIRE(world.trucks[0].haulJobId != 0);
}

TEST_CASE("escort travels to the load system instead of jumping onto the delivery hop", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 1);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 0, 0, CommandSource::Player)));

	while (world.trucks[0].haulJobId == 0)
	{
		REQUIRE(world.tick < 40);
		stepGalaxy(world);
	}

	while (world.trucks[0].haulJobId != 0)
	{
		REQUIRE(world.tick < 80);
		stepGalaxy(world);
	}

	while (world.trucks[0].haulJobId == 0)
	{
		REQUIRE(world.tick < 120);
		stepGalaxy(world);
	}

	const Fleet* home = world.fleetById(1);
	const Convoy* standing = world.convoyById(home->escortConvoyId);
	REQUIRE(home != nullptr);
	REQUIRE(standing != nullptr);
	if (home->systemId != standing->loadSystemId)
	{
		REQUIRE(home->motion.route.size() >= 2);
		REQUIRE(home->motion.route.front() == home->systemId);
		REQUIRE(home->motion.route.back() == standing->loadSystemId);
	}
}

TEST_CASE("clear escort releases assembling trucks", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 2);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 3, 0, 0, CommandSource::Player)));
	stepGalaxy(world);
	const long long standingId = world.fleetById(1)->escortConvoyId;
	REQUIRE(countTrucksInConvoy(world, standingId) == 2);

	REQUIRE(execute(world, GameCommand::setFleetEscort(1, false, 0, 0, 0, CommandSource::Player)));
	REQUIRE(world.fleetById(1)->escortConvoyId == 0);
	REQUIRE(world.convoyById(standingId) == nullptr);
	REQUIRE(world.trucks[0].convoyId == 0);
	REQUIRE(world.trucks[1].convoyId == 0);
}

TEST_CASE("escorted trucks use the convoy hop, not a parked system offset", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 1);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 0, 0, CommandSource::Player)));

	const long long standingId = world.fleetById(1)->escortConvoyId;
	while (!convoyIsUnderway(*world.convoyById(standingId)))
	{
		REQUIRE(world.tick < 40);
		stepGalaxy(world);
	}

	const auto truckPos = truckMarkerPosition(world, world.trucks[0]);
	const auto onLane = laneWorldPosition(world, world.convoyById(standingId)->motion);
	REQUIRE(truckPos.has_value());
	REQUIRE(onLane.has_value());
	const Convoy* standing = world.convoyById(standingId);
	glm::vec2 expected = *onLane;
	if (standing->motion.progress > 0.0f && standing->motion.progress < 1.0f)
	{
		long long fromId = 0;
		long long toId = 0;
		REQUIRE(hopEndpoints(standing->motion, fromId, toId));
		const glm::vec2 along = glm::normalize(
			world.systemById(toId)->position - world.systemById(fromId)->position);
		expected = *onLane - along * (GalaxyFleetSize * 0.5f + GalaxyTruckSize * 0.5f + 4.0f);
	}

	REQUIRE(glm::distance(*truckPos, expected) == Catch::Approx(0.0f));
}

TEST_CASE("joining standing trucks stay on their own hop with no escort trail", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 2);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 3, 0, 0, CommandSource::Player)));
	stepGalaxy(world);

	const Convoy* standing = world.convoyById(world.fleetById(1)->escortConvoyId);
	REQUIRE(standing != nullptr);
	REQUIRE(convoyIsStanding(*standing));
	REQUIRE_FALSE(convoyIsUnderway(*standing));
	REQUIRE(world.trucks[0].convoyId == standing->id);

	world.trucks[0].motion.route = shortestLanePath(world, 5, 3);
	world.trucks[0].motion.hop = 0;
	world.trucks[0].motion.progress = 0.5f;
	REQUIRE(world.trucks[0].motion.route.size() >= 2);

	const auto truckPos = truckMarkerPosition(world, world.trucks[0]);
	const auto onHop = laneWorldPosition(world, world.trucks[0].motion);
	REQUIRE(truckPos.has_value());
	REQUIRE(onHop.has_value());
	REQUIRE(glm::distance(*truckPos, *onHop) == Catch::Approx(0.0f));
}

TEST_CASE("move fleet while escorting a run fails", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 1);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 0, 0, CommandSource::Player)));

	while (!convoyIsUnderway(*world.convoyById(world.fleetById(1)->escortConvoyId)))
	{
		REQUIRE(world.tick < 40);
		stepGalaxy(world);
	}

	REQUIRE_FALSE(execute(world, GameCommand::moveFleetToSystem(1, 1, CommandSource::Player)));
}

TEST_CASE("move fleet while assembling relocates the rally", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 2);
	parkFleetAt(world, 1, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 3, 0, 0, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.fleetById(1)->escortConvoyId != 0);
	REQUIRE(world.convoyById(world.fleetById(1)->escortConvoyId)->haulJobId == 0);

	REQUIRE(execute(world, GameCommand::moveFleetToSystem(1, 3, CommandSource::Player)));
	REQUIRE(world.fleets.front().motion.route.back() == 3);

	stepGalaxy(world);

	REQUIRE(world.trucks[0].motion.route.back() == 3);
	REQUIRE(world.trucks[1].motion.route.back() == 3);
}

TEST_CASE("job-group dump still dissolves", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 1);

	stepGalaxy(world);

	REQUIRE(world.trucks[0].convoyId != 0);
	REQUIRE_FALSE(convoyIsStanding(*world.convoyById(world.trucks[0].convoyId)));
	const long long jobGroupId = world.trucks[0].convoyId;
	while (world.trucks[0].haulJobId != 0)
	{
		REQUIRE(world.tick < 80);
		stepGalaxy(world);
	}

	REQUIRE(world.trucks[0].convoyId == 0);
	REQUIRE(world.convoyById(jobGroupId) == nullptr);
}

TEST_CASE("Ashreach escort does not take Helion trucks", "[slr][Escort]")
{
	WorldData world = makeStarterMap();
	keepFirstEmptyTrucks(world, 2);
	parkFleetAt(world, 2, 5);
	REQUIRE(execute(world, GameCommand::setFleetEscort(2, true, 2, 0, 0, CommandSource::Player)));

	stepGalaxy(world);

	const Convoy* standing = world.convoyById(world.fleetById(2)->escortConvoyId);
	REQUIRE(standing != nullptr);
	REQUIRE(countTrucksInConvoy(world, standing->id) == 0);
}

}
}
