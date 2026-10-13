#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace YardTruckTests
{

	long long fleetHullCount(const WorldData& world)
	{
		long long count = 0;
		for (const auto& fleet : world.fleets)
		{
			for (const auto& row : fleet.composition)
			{
				count += row.count;
			}
		}

		return count;
	}

	void setSteelPile(StarSystem& system, long long amount)
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
		world.haulJobs.clear();
		for (auto& system : world.systems)
		{
			for (auto& stock : system.stocks)
			{
				stock.need = 0;
			}
		}

		Yard* helionYard = yardAtSystem(world, 1);
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

	long long countTrucksOfClass(const WorldData& world, long long classId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (truck.classId == classId)
			{
				++count;
			}
		}

		return count;
	}

	const Truck* lastTruck(const WorldData& world)
	{
		if (world.trucks.empty())
		{
			return nullptr;
		}

		return &world.trucks.back();
	}

TEST_CASE("StartYardBuild appends a truck class without spending steel", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 400);
	const long long trucksBefore = static_cast<long long>(world.trucks.size());

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 400);
	REQUIRE(yardAtSystem(world, 1)->hullBuildQueue.size() == 1);
	REQUIRE(yardAtSystem(world, 1)->hullBuildQueue.front() == HelionCivilianTruckClassId);
	REQUIRE(world.trucks.size() == static_cast<size_t>(trucksBefore));
}

TEST_CASE("StartYardBuild rejects a combat class and a rival truck class", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	REQUIRE_FALSE(execute(world, GameCommand::startYardBuild(1, 1, CommandSource::Player)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::startYardBuild(1, AshreachCivilianTruckClassId, CommandSource::Player)));
	REQUIRE(yardAtSystem(world, 1)->hullBuildQueue.empty());
}

TEST_CASE("a second queued hull stays on the yard until the first finishes", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 200 + TruckHullSteelCost * 2);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionMilitaryTruckClassId, CommandSource::Player)));

	stepGalaxy(world);

	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(helionYard != nullptr);
	REQUIRE(helionYard->hullBuildQueue.size() == 1);
	REQUIRE(helionYard->hullBuildQueue.front() == HelionMilitaryTruckClassId);
	REQUIRE(slipById(*helionYard, 1)->hullClassId == HelionCivilianTruckClassId);
	REQUIRE(slipById(*helionYard, 1)->remainingHullBuildTicks == TruckHullBuildTicks - 1);
	REQUIRE(world.trucks.empty());

	for (int tick = 0; tick < TruckHullBuildTicks - 1; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(countTrucksOfClass(world, HelionCivilianTruckClassId) == 1);
	REQUIRE(countTrucksOfClass(world, HelionMilitaryTruckClassId) == 0);
	REQUIRE(helionYard->hullBuildQueue.size() == 1);
	REQUIRE(slipById(*helionYard, 1)->hullClassId == 0);
}

TEST_CASE("two finished slips pull two jobs; one steel slice advances only the lower id", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(helionYard != nullptr);
	helionYard->slips.push_back(YardSlip{ .id = 3, .remainingBuildTicks = 0 });

	const long long slice = hullSteelPerTick(*world.shipClassById(HelionCivilianTruckClassId));
	REQUIRE(slice == TruckHullSteelCost / TruckHullBuildTicks);
	setSteelPile(*helion, YardPopulationReserved_Steel + slice);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionMilitaryTruckClassId, CommandSource::Player)));

	stepGalaxy(world);

	REQUIRE(slipById(*helionYard, 1)->hullClassId == HelionCivilianTruckClassId);
	REQUIRE(slipById(*helionYard, 1)->remainingHullBuildTicks == TruckHullBuildTicks - 1);
	REQUIRE(slipById(*helionYard, 3)->hullClassId == HelionMilitaryTruckClassId);
	REQUIRE(slipById(*helionYard, 3)->remainingHullBuildTicks == TruckHullBuildTicks);
	REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 0);
}

TEST_CASE("two steel slices advance both busy slips", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(helionYard != nullptr);
	helionYard->slips.push_back(YardSlip{ .id = 3, .remainingBuildTicks = 0 });

	const long long slice = hullSteelPerTick(*world.shipClassById(HelionCivilianTruckClassId));
	setSteelPile(*helion, YardPopulationReserved_Steel + slice * 2);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionMilitaryTruckClassId, CommandSource::Player)));

	stepGalaxy(world);

	REQUIRE(slipById(*helionYard, 1)->remainingHullBuildTicks == TruckHullBuildTicks - 1);
	REQUIRE(slipById(*helionYard, 3)->remainingHullBuildTicks == TruckHullBuildTicks - 1);
}

TEST_CASE("hull ticks do not eat the population reserved floor", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 250);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	stepGalaxy(world);

	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 250);
	REQUIRE(slipById(*helionYard, 1)->remainingHullBuildTicks == TruckHullBuildTicks);
	REQUIRE(world.trucks.empty());
}

TEST_CASE("a starved hull waits until steel covers a tick then finishes", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	const long long slice = hullSteelPerTick(*world.shipClassById(HelionCivilianTruckClassId));
	setSteelPile(*helion, YardPopulationReserved_Steel + slice);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(slipById(*yardAtSystem(world, 1), 1)->remainingHullBuildTicks == TruckHullBuildTicks - 1);

	stepGalaxy(world);
	REQUIRE(slipById(*yardAtSystem(world, 1), 1)->remainingHullBuildTicks == TruckHullBuildTicks - 1);
	REQUIRE(world.trucks.empty());

	setSteelPile(*helion, YardPopulationReserved_Steel + slice * (TruckHullBuildTicks - 1));
	for (int tick = 0; tick < TruckHullBuildTicks - 1; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(countTrucksOfClass(world, HelionCivilianTruckClassId) == 1);
	const Truck* built = lastTruck(world);
	REQUIRE(built != nullptr);
	REQUIRE(built->classId == HelionCivilianTruckClassId);
	REQUIRE_FALSE(truckHasRole(world, *built, HullRole::MilitaryCargo));
	REQUIRE(truckSystemId(*built) == 1);
	REQUIRE(built->cargoAmount == 0);
}

TEST_CASE("finished military truck copies class id and isMilitary", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	const long long hullsBefore = fleetHullCount(world);
	setSteelPile(*helion, YardPopulationReserved_Steel + TruckHullSteelCost);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionMilitaryTruckClassId, CommandSource::Player)));
	for (int tick = 0; tick < TruckHullBuildTicks; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(countTrucksOfClass(world, HelionMilitaryTruckClassId) == 1);
	const Truck* built = lastTruck(world);
	REQUIRE(built != nullptr);
	REQUIRE(truckHasRole(world, *built, HullRole::MilitaryCargo));
	REQUIRE(built->classId == HelionMilitaryTruckClassId);
	REQUIRE(fleetHullCount(world) == hullsBefore);
}

TEST_CASE("eat ticks post deferred need", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	const long long slice = hullSteelPerTick(*world.shipClassById(HelionCivilianTruckClassId));
	setSteelPile(*helion, YardPopulationReserved_Steel + slice);

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(helion->stocks.front().deferredNeed == slice);
}

TEST_CASE("a slip still constructing its bay does not pull a hull", "[slr][YardTruck]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(helionYard != nullptr);
	helionYard->slips.front().remainingBuildTicks = 4;

	REQUIRE(execute(
		world,
		GameCommand::startYardBuild(1, HelionCivilianTruckClassId, CommandSource::Player)));
	stepGalaxy(world);

	REQUIRE(helionYard->hullBuildQueue.size() == 1);
	REQUIRE(helionYard->slips.front().hullClassId == 0);
	REQUIRE(world.trucks.empty());
}

}
}
