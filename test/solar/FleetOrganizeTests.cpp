#include <World/GameCommand.hpp>
#include <World/StarterMap.hpp>
#include <World/SystemPlaces.hpp>
#include <algorithm>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace FleetOrganizeTests
{

	constexpr long long HelionSystemId = 1;
	constexpr long long NikeClassId = 1;
	constexpr long long DestroyerClassId = 2;
	constexpr long long HomeFleetId = 1;
	constexpr long long AshreachFleetId = 2;

	long long countHullsOfClass(const Fleet& fleet, long long classId)
	{
		long long count = 0;
		for (const auto& hull : fleet.hulls)
		{
			if (hull.classId == classId)
			{
				++count;
			}
		}

		return count;
	}

	const Fleet* fleetByName(const WorldData& world, const char* name)
	{
		for (const auto& fleet : world.fleets)
		{
			if (fleet.name == name)
			{
				return &fleet;
			}
		}

		return nullptr;
	}

	Fleet& addParkedFleetAtHelionYard(WorldData& world, const std::string& name)
	{
		world.fleets.push_back(Fleet{
			.id = nextFleetId(world),
			.empireId = HelionEmpireId,
			.name = name,
			.systemId = HelionSystemId,
			.placeId = HelionYardPlaceId,
			.motion = { .speed = 50.0f } });
		return world.fleets.back();
	}

TEST_CASE("split peels lowest hull ids and leaves at least one hull on source", "[slr][FleetOrganize]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	const long long nikesBefore = countHullsOfClass(*home, NikeClassId);
	const long long fleetsBefore = static_cast<long long>(world.fleets.size());

	REQUIRE(execute(
		world,
		GameCommand::splitFleetByClass(HomeFleetId, NikeClassId, 2, CommandSource::Player)));

	home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	REQUIRE(static_cast<long long>(world.fleets.size()) == fleetsBefore + 1);
	REQUIRE(countHullsOfClass(*home, NikeClassId) == nikesBefore - 2);
	REQUIRE(fleetShipCount(*home) >= 1);

	const Fleet* splitFleet = fleetByName(world, "Home Fleet split");
	REQUIRE(splitFleet != nullptr);
	REQUIRE(countHullsOfClass(*splitFleet, NikeClassId) == 2);
	REQUIRE(splitFleet->systemId == HelionSystemId);
	REQUIRE(splitFleet->placeId == HelionYardPlaceId);

	std::vector<long long> splitHullIds;
	for (const auto& hull : splitFleet->hulls)
	{
		if (hull.classId == NikeClassId)
		{
			splitHullIds.push_back(hull.id);
		}
	}

	std::sort(splitHullIds.begin(), splitHullIds.end());
	REQUIRE(splitHullIds.size() == 2);
	REQUIRE(splitHullIds[0] < splitHullIds[1]);
}

TEST_CASE("split rejects peeling every hull and rival fleets", "[slr][FleetOrganize]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	const long long total = fleetShipCount(*home);
	REQUIRE_FALSE(execute(
		world,
		GameCommand::splitFleetByClass(HomeFleetId, NikeClassId, total, CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::splitFleetByClass(AshreachFleetId, NikeClassId, 1, CommandSource::Player)));
}

TEST_CASE("merge combines fleets at the same place and removes the higher fleet id", "[slr][FleetOrganize]")
{
	WorldData world = makeStarterMap();
	Fleet& detachment = addParkedFleetAtHelionYard(world, "Detachment");
	appendEmptyMagazineHulls(world, detachment, NikeClassId, 2);
	const long long detachmentId = detachment.id;
	const long long survivorId = std::min(HomeFleetId, detachmentId);
	const long long loserId = std::max(HomeFleetId, detachmentId);

	REQUIRE(execute(
		world,
		GameCommand::mergeFleets(HomeFleetId, detachmentId, CommandSource::Player)));

	REQUIRE(world.fleetById(loserId) == nullptr);
	const Fleet* survivor = world.fleetById(survivorId);
	REQUIRE(survivor != nullptr);
	REQUIRE(countHullsOfClass(*survivor, NikeClassId) == 10);
}

TEST_CASE("merge rejects different places and standing escort", "[slr][FleetOrganize]")
{
	WorldData world = makeStarterMap();
	Fleet* kestrel = world.fleetById(3);
	REQUIRE(kestrel != nullptr);
	REQUIRE_FALSE(execute(
		world,
		GameCommand::mergeFleets(HomeFleetId, kestrel->id, CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world, GameCommand::mergeFleets(HomeFleetId, HomeFleetId, CommandSource::Player)));

	Fleet& detachment = addParkedFleetAtHelionYard(world, "Detachment");
	appendEmptyMagazineHulls(world, detachment, NikeClassId, 1);
	REQUIRE(execute(
		world,
		GameCommand::setFleetEscort(HomeFleetId, true, 1, 0, 0, CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::mergeFleets(HomeFleetId, detachment.id, CommandSource::Player)));
}

TEST_CASE("rename trims whitespace and rejects empty names", "[slr][FleetOrganize]")
{
	WorldData world = makeStarterMap();
	REQUIRE(execute(
		world, GameCommand::renameFleet(HomeFleetId, "  Rim picket  ", CommandSource::Player)));
	const Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	REQUIRE(home->name == "Rim picket");
	REQUIRE_FALSE(execute(world, GameCommand::renameFleet(HomeFleetId, "   ", CommandSource::Player)));
}

TEST_CASE("split commands are deterministic", "[slr][FleetOrganize]")
{
	WorldData a = makeStarterMap();
	WorldData b = makeStarterMap();
	REQUIRE(execute(
		a, GameCommand::splitFleetByClass(HomeFleetId, NikeClassId, 1, CommandSource::Player)));
	REQUIRE(execute(
		b, GameCommand::splitFleetByClass(HomeFleetId, NikeClassId, 1, CommandSource::Player)));

	const Fleet* splitA = fleetByName(a, "Home Fleet split");
	const Fleet* splitB = fleetByName(b, "Home Fleet split");
	REQUIRE(splitA != nullptr);
	REQUIRE(splitB != nullptr);
	REQUIRE(splitA->hulls.size() == splitB->hulls.size());
	for (size_t i = 0; i < splitA->hulls.size(); ++i)
	{
		REQUIRE(splitA->hulls[i].id == splitB->hulls[i].id);
		REQUIRE(splitA->hulls[i].classId == splitB->hulls[i].classId);
	}
}

}
}
