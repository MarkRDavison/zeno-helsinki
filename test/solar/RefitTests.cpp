#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace RefitTests
{

	constexpr long long HelionSystemId = 1;
	constexpr long long KestrelSystemId = 2;
	constexpr long long NikeClassId = 1;
	constexpr long long DestroyerClassId = 2;
	constexpr long long LanceClassId = 3;
	constexpr long long HomeFleetId = 1;

	void setSteelPile(StarSystem& system, GoodAmount amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == SteelGoodId)
			{
				stock.goodAmount = amount;
				stock.inbound = 0;
				stock.need = 0;
				stock.deferredNeed = 0;
				stock.deferredNeedHeldOneTick = false;
				return;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = amount });
	}

	void silenceHelionTrafficAndPopulation(WorldData& world)
	{
		world.trucks.clear();
		world.convoys.clear();
		Yard* helionYard = yardAtSystem(world, HelionSystemId);
		REQUIRE(helionYard != nullptr);
		helionYard->populationConsumePerTick.clear();
	}

	YardSlip* slipById(Yard& yard, long long slipId)
	{
		for (auto& slip : yard.slips)
		{
			if (slip.id == slipId)
			{
				return &slip;
			}
		}

		return nullptr;
	}

	long long lowestHullIdOfClass(const Fleet& fleet, long long classId)
	{
		long long hullId = 0;
		for (const auto& hull : fleet.hulls)
		{
			if (hull.classId != classId)
			{
				continue;
			}

			if (hullId == 0 || hull.id < hullId)
			{
				hullId = hull.id;
			}
		}

		return hullId;
	}

	const FleetHull* hullById(const Fleet& fleet, long long hullId)
	{
		for (const auto& hull : fleet.hulls)
		{
			if (hull.id == hullId)
			{
				return &hull;
			}
		}

		return nullptr;
	}

	GameCommand startDestroyerToNike()
	{
		return GameCommand::startYardRefit(
			HelionSystemId,
			DestroyerClassId,
			NikeClassId,
			CommandSource::Player);
	}

TEST_CASE("StartYardRefit removes the hull from the fleet onto the yard", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	const long long hullId = lowestHullIdOfClass(*home, DestroyerClassId);
	REQUIRE(hullId != 0);
	REQUIRE(classCountFor(home->composition, DestroyerClassId) == 12);
	REQUIRE(classCountFor(home->composition, NikeClassId) == 8);

	REQUIRE(execute(world, startDestroyerToNike()));
	REQUIRE(classCountFor(home->composition, DestroyerClassId) == 11);
	REQUIRE(hullById(*home, hullId) == nullptr);
	Yard* helionYard = yardAtSystem(world, HelionSystemId);
	REQUIRE(helionYard != nullptr);
	REQUIRE(helionYard->hullRefitQueue.size() == 1);
	REQUIRE(helionYard->hullRefitQueue.front().hullId == hullId);
	REQUIRE(helionYard->hullRefitQueue.front().fromClassId == DestroyerClassId);
	REQUIRE(helionYard->hullRefitQueue.front().toClassId == NikeClassId);
	REQUIRE(helionYard->hullRefitQueue.front().preferredFleetId == HomeFleetId);
}

TEST_CASE("a Destroyer becomes a Nike after 50 ticks with empty magazines", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, YardPopulationReserved_Steel + RefitHullSteelCost);
	for (auto& stock : helion->stocks)
	{
		if (stock.goodId == MissileGoodId)
		{
			stock.goodAmount = 0;
		}
	}

	Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	const long long hullId = lowestHullIdOfClass(*home, DestroyerClassId);

	REQUIRE(execute(world, startDestroyerToNike()));
	for (int tick = 0; tick < RefitHullBuildTicks; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(classCountFor(home->composition, DestroyerClassId) == 11);
	REQUIRE(classCountFor(home->composition, NikeClassId) == 9);
	const FleetHull* hull = hullById(*home, hullId);
	REQUIRE(hull != nullptr);
	REQUIRE(hull->classId == NikeClassId);
	REQUIRE(magazineUsed(hull->magazine) == 0.f);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == YardPopulationReserved_Steel);
	REQUIRE(yardAtSystem(world, HelionSystemId)->hullRefitQueue.empty());
	REQUIRE(slipById(*yardAtSystem(world, HelionSystemId), 1)->hullClassId == 0);
}

TEST_CASE("a fleet that leaves mid-refit does not take the finished hull", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, YardPopulationReserved_Steel + RefitHullSteelCost);
	Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	const long long hullId = lowestHullIdOfClass(*home, DestroyerClassId);

	REQUIRE(execute(world, startDestroyerToNike()));
	REQUIRE(execute(
		world,
		GameCommand::moveFleetToSystem(HomeFleetId, KestrelSystemId, CommandSource::Player)));
	for (int tick = 0; tick < RefitHullBuildTicks; ++tick)
	{
		stepGalaxy(world);
	}

	home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	REQUIRE(home->systemId == KestrelSystemId);
	REQUIRE(fleetIsParked(*home));
	REQUIRE(hullById(*home, hullId) == nullptr);
	REQUIRE(classCountFor(home->composition, NikeClassId) == 8);

	bool foundAtHelion = false;
	for (const auto& fleet : world.fleets)
	{
		if (fleet.systemId != HelionSystemId || !fleetIsParked(fleet))
		{
			continue;
		}

		const FleetHull* hull = hullById(fleet, hullId);
		if (hull == nullptr)
		{
			continue;
		}

		foundAtHelion = true;
		REQUIRE(hull->classId == NikeClassId);
		REQUIRE(fleet.empireId == HelionEmpireId);
	}

	REQUIRE(foundAtHelion);
}

TEST_CASE("refit steel waits when the reserved floor is all that remains", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, YardPopulationReserved_Steel);
	REQUIRE(execute(world, startDestroyerToNike()));
	stepGalaxy(world);

	YardSlip* slip = slipById(*yardAtSystem(world, HelionSystemId), 1);
	REQUIRE(slip != nullptr);
	REQUIRE(slip->refitHullId != 0);
	REQUIRE(slip->remainingHullBuildTicks == RefitHullBuildTicks);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == YardPopulationReserved_Steel);
}

TEST_CASE("StartYardRefit rejects illegal class pairs and missing hulls", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	world.shipClasses.push_back(ShipClass{
		.id = 99,
		.empireId = HelionEmpireId,
		.name = "Monitor",
		.hullRoles = hullRoleBit(HullRole::Warship),
		.hullSize = HullSize::Medium });

	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardRefit(
			HelionSystemId,
			DestroyerClassId,
			99,
			CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardRefit(
			HelionSystemId,
			DestroyerClassId,
			HelionCivilianTruckClassId,
			CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardRefit(
			HelionSystemId,
			DestroyerClassId,
			LanceClassId,
			CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardRefit(
			HelionSystemId,
			DestroyerClassId,
			DestroyerClassId,
			CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardRefit(
			KestrelSystemId,
			DestroyerClassId,
			NikeClassId,
			CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardRefit(
			HelionSystemId,
			LanceClassId,
			NikeClassId,
			CommandSource::Player)));
	REQUIRE(yardAtSystem(world, HelionSystemId)->hullRefitQueue.empty());
}

TEST_CASE("StartYardRefit rejects an in-transit fleet", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	REQUIRE(execute(
		world,
		GameCommand::moveFleetToSystem(HomeFleetId, KestrelSystemId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE_FALSE(fleetIsParked(*world.fleetById(HomeFleetId)));
	REQUIRE_FALSE(execute(world, startDestroyerToNike()));
}

TEST_CASE("two slips spend a refit slice each on the same tick", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	Yard* helionYard = yardAtSystem(world, HelionSystemId);
	REQUIRE(helionYard != nullptr);
	helionYard->slips.push_back(YardSlip{ .id = 3, .remainingBuildTicks = 0 });
	const long long slice = refitSteelPerTick();
	REQUIRE(slice == RefitHullSteelCost / RefitHullBuildTicks);
	setSteelPile(*helion, YardPopulationReserved_Steel + slice * 2);

	REQUIRE(execute(world, startDestroyerToNike()));
	REQUIRE(execute(world, startDestroyerToNike()));
	stepGalaxy(world);

	REQUIRE(slipById(*helionYard, 1)->remainingHullBuildTicks == RefitHullBuildTicks - 1);
	REQUIRE(slipById(*helionYard, 3)->remainingHullBuildTicks == RefitHullBuildTicks - 1);
	REQUIRE(slipById(*helionYard, 1)->refitHullId != 0);
	REQUIRE(slipById(*helionYard, 3)->refitHullId != 0);
	REQUIRE(stockpileAvailableForYardConsume(world, HelionSystemId, SteelGoodId) == 0);
}

TEST_CASE("a busy new-build slip does not pull a waiting refit", "[slr][Refit]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, YardPopulationReserved_Steel + TruckHullSteelCost + RefitHullSteelCost);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(
			HelionSystemId,
			HelionCivilianTruckClassId,
			CommandSource::Player)));
	stepGalaxy(world);
	Yard* helionYard = yardAtSystem(world, HelionSystemId);
	REQUIRE(slipById(*helionYard, 1)->hullClassId == HelionCivilianTruckClassId);
	REQUIRE(slipById(*helionYard, 1)->refitHullId == 0);

	REQUIRE(execute(world, startDestroyerToNike()));
	stepGalaxy(world);

	REQUIRE(helionYard->hullRefitQueue.size() == 1);
	REQUIRE(slipById(*helionYard, 1)->hullClassId == HelionCivilianTruckClassId);
	REQUIRE(slipById(*helionYard, 1)->refitHullId == 0);
}

}
}
