#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <PutAtJump.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace MilitaryLiftTests
{

	void setStockpile(StarSystem& system, long long goodId, GoodAmount amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				stock.goodAmount = amount;
				return;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = goodId, .goodAmount = amount });
	}

	void clearTraffic(WorldData& world)
	{
		world.trucks.clear();
		world.convoys.clear();
		StarSystem* helion = world.systemById(1);
		REQUIRE(helion != nullptr);
		for (auto& stock : helion->stocks)
		{
			stock.inbound = 0;
		}
	}

	Truck& addParkedTruck(
		WorldData& world,
		long long id,
		long long systemId,
		bool isMilitary)
	{
		world.trucks.push_back(Truck{
			.id = id,
			.empireId = HelionEmpireId,
			.motion = { .route = { systemId }, .speed = 25.0f },
			.placeId = parkedInstallationPlaceId(world, systemId),
			.classId = isMilitary ? HelionMilitaryTruckClassId : HelionCivilianTruckClassId });
		return world.trucks.back();
	}

	void putSteelAtJunction(WorldData& world)
	{
		StarSystem* junction = world.systemById(3);
		REQUIRE(junction != nullptr);
		setStockpile(*junction, SteelGoodId, TruckCapacity_Steel);
	}

TEST_CASE("starter seeds sixteen civilians and two military trucks at Helion", "[slr][MilitaryLift]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(
		world.trucks.size() ==
		StarterCivilianTruckCount + StarterMilitaryTruckCount + StarterTenderCount);
	long long civilian = 0;
	long long military = 0;
	long long tenders = 0;
	for (const auto& truck : world.trucks)
	{
		if (truckHasRole(world, truck, HullRole::Tender))
		{
			++tenders;
			REQUIRE(isTruckAtSystem(truck, 1));
		}
		else if (truckHasRole(world, truck, HullRole::MilitaryCargo))
		{
			++military;
			REQUIRE(isTruckAtSystem(truck, 1));
		}
		else
		{
			++civilian;
		}
	}
	REQUIRE(civilian == StarterCivilianTruckCount);
	REQUIRE(military == StarterMilitaryTruckCount);
	REQUIRE(tenders == StarterTenderCount);
}

TEST_CASE("at war civilians haul Helion steel through Kestrel", "[slr][MilitaryLift]")
{
	WorldData world = makeStarterMap();
	clearTraffic(world);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	putSteelAtJunction(world);
	Truck& civilian = addParkedTruck(world, 10, 5, false);
	const HaulJob* steel = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(steel != nullptr);
	REQUIRE(civilianLanePath(world, 3, 1, HelionEmpireId) == shortestLanePath(world, 3, 1));

	REQUIRE(execute(world, GameCommand::acceptHaulJob(civilian.id, steel->id, CommandSource::Ai)));
	REQUIRE(civilian.haulJobId == steel->id);
	stepUntilTruckHasGalaxyRoute(world, civilian.id);
	REQUIRE(civilian.motion.route == haulLanePath(world, 5, 3, HelionEmpireId, false, false));

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	const long long inboundBeforeLoad = getInbound(*helion, SteelGoodId);
	int ticks = 0;
	while (civilian.cargoAmount == 0 && ticks < 200)
	{
		stepGalaxy(world);
		++ticks;
	}
	REQUIRE(civilian.cargoAmount == TruckCapacity_Steel);
	REQUIRE(getInbound(*helion, SteelGoodId) == inboundBeforeLoad + TruckCapacity_Steel);

	while (civilian.haulJobId != 0 && ticks < 400)
	{
		stepGalaxy(world);
		++ticks;
	}
	REQUIRE(civilian.haulJobId == 0);
	REQUIRE(civilian.cargoAmount == 0);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) >= TruckCapacity_Steel);
	REQUIRE(getInbound(*helion, SteelGoodId) == inboundBeforeLoad);
}

TEST_CASE("parked enemy on Kestrel blocks military lift", "[slr][MilitaryLift]")
{
	WorldData world = makeStarterMap();
	clearTraffic(world);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	putSteelAtJunction(world);
	Fleet* rival = world.fleetById(2);
	REQUIRE(rival != nullptr);
	rival->systemId = 2;
	rival->motion = {};
	addParkedTruck(world, 17, 1, true);
	const HaulJob* steel = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(steel != nullptr);
	REQUIRE(haulLanePath(world, 1, 3, HelionEmpireId, false, true).empty());
	REQUIRE_FALSE(execute(world, GameCommand::acceptHaulJob(17, steel->id, CommandSource::Ai)));
}

TEST_CASE("forbid Ashreach in peace does not block Helion steel", "[slr][MilitaryLift]")
{
	WorldData world = makeStarterMap();
	clearTraffic(world);
	REQUIRE(execute(
		world,
		GameCommand::setCivilianHaulForbidden(
			HelionEmpireId,
			AshreachEmpireId,
			true,
			CommandSource::Player)));
	putSteelAtJunction(world);
	addParkedTruck(world, 10, 5, false);
	const HaulJob* steel = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(steel != nullptr);
	REQUIRE(execute(world, GameCommand::acceptHaulJob(10, steel->id, CommandSource::Ai)));
}

TEST_CASE("military empty still joins a standing escort", "[slr][MilitaryLift]")
{
	WorldData world = makeStarterMap();
	clearTraffic(world);
	addParkedTruck(world, 10, 5, false);
	addParkedTruck(world, 17, 5, true);
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 5;
	home->motion.route.clear();
	home->motion.hop = 0;
	home->motion.progress = 0.0f;
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 1, 0, CommandSource::Player)));

	stepGalaxy(world);

	const long long standingId = world.fleetById(1)->escortConvoyId;
	REQUIRE(standingId != 0);
	REQUIRE(countCivilianTrucksInConvoy(world, standingId) == 1);
	REQUIRE(countMilitaryTrucksInConvoy(world, standingId) == 1);
	REQUIRE(world.truckById(10)->convoyId == standingId);
	REQUIRE(world.truckById(17)->convoyId == standingId);
	REQUIRE(truckHasRole(world, *world.truckById(17), HullRole::MilitaryCargo));
}

TEST_CASE("military-only standing convoy does not take civilian trucks", "[slr][MilitaryLift]")
{
	WorldData world = makeStarterMap();
	clearTraffic(world);
	addParkedTruck(world, 10, 5, false);
	addParkedTruck(world, 17, 5, true);
	addParkedTruck(world, 18, 5, true);
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 5;
	home->motion.route.clear();
	home->motion.hop = 0;
	home->motion.progress = 0.0f;
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 0, 2, 0, CommandSource::Player)));

	stepGalaxy(world);

	const Convoy* standing = world.convoyById(world.fleetById(1)->escortConvoyId);
	REQUIRE(standing != nullptr);
	REQUIRE(standingConvoyUsesMilitaryHaulPath(*standing));
	REQUIRE(countCivilianTrucksInConvoy(world, standing->id) == 0);
	REQUIRE(countMilitaryTrucksInConvoy(world, standing->id) == 2);
	REQUIRE(world.truckById(10)->convoyId != standing->id);
}

}
}
