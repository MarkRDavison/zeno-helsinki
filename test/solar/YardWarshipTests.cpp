#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace YardWarshipTests
{

	constexpr long long HelionSystemId = 1;
	constexpr long long NikeClassId = 1;
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
		world.haulJobs.clear();
		for (auto& system : world.systems)
		{
			for (auto& stock : system.stocks)
			{
				stock.need = 0;
			}
		}

		Yard* helionYard = yardAtSystem(world, HelionSystemId);
		REQUIRE(helionYard != nullptr);
		helionYard->populationConsumePerTick.clear();
	}

	long long hullCountOfClass(const Fleet& fleet, long long classId)
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

	const FleetHull* newestHullOfClass(const Fleet& fleet, long long classId)
	{
		const FleetHull* newest = nullptr;
		for (const auto& hull : fleet.hulls)
		{
			if (hull.classId != classId)
			{
				continue;
			}

			if (newest == nullptr || hull.id > newest->id)
			{
				newest = &hull;
			}
		}

		return newest;
	}

TEST_CASE("StartYardBuild accepts a warship class without spending steel on enqueue", "[slr][YardWarship]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 5000);

	REQUIRE(execute(world, GameCommand::startYardBuild(HelionSystemId, NikeClassId, CommandSource::Player)));
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 5000);
	Yard* helionYard = yardAtSystem(world, HelionSystemId);
	REQUIRE(helionYard != nullptr);
	REQUIRE(helionYard->hullBuildQueue.size() == 1);
	REQUIRE(helionYard->hullBuildQueue.front() == NikeClassId);
}

TEST_CASE("StartYardBuild rejects a rival warship class", "[slr][YardWarship]")
{
	WorldData world = makeStarterMap();
	REQUIRE_FALSE(
		execute(world, GameCommand::startYardBuild(HelionSystemId, LanceClassId, CommandSource::Player)));
	REQUIRE(yardAtSystem(world, HelionSystemId)->hullBuildQueue.empty());
}

TEST_CASE("a finished warship build adds a full-magazine hull to a parked fleet", "[slr][YardWarship]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	const ShipClass* nikeClass = world.shipClassById(NikeClassId);
	REQUIRE(nikeClass != nullptr);
	const long long buildTicks = nikeClass->buildTicks;
	REQUIRE(buildTicks > 0);
	const GoodAmount slice = hullSteelPerTick(*nikeClass);
	setSteelPile(*helion, YardPopulationReserved_Steel + slice * static_cast<GoodAmount>(buildTicks));

	Fleet* home = world.fleetById(HomeFleetId);
	REQUIRE(home != nullptr);
	const long long nikesBefore = hullCountOfClass(*home, NikeClassId);

	REQUIRE(execute(world, GameCommand::startYardBuild(HelionSystemId, NikeClassId, CommandSource::Player)));

	for (long long tick = 0; tick < buildTicks; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(hullCountOfClass(*home, NikeClassId) == nikesBefore + 1);
	REQUIRE(classCountFor(home->composition, NikeClassId) == nikesBefore + 1);
	const FleetHull* built = newestHullOfClass(*home, NikeClassId);
	REQUIRE(built != nullptr);
	REQUIRE(fleetMissileAmmoForClass(*home, NikeClassId) >= nikeClass->magazineSize);
}

TEST_CASE("warship build steel waits when only the reserved floor remains", "[slr][YardWarship]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(HelionSystemId);
	REQUIRE(helion != nullptr);
	const ShipClass* nikeClass = world.shipClassById(NikeClassId);
	REQUIRE(nikeClass != nullptr);
	setSteelPile(*helion, YardPopulationReserved_Steel);

	REQUIRE(execute(world, GameCommand::startYardBuild(HelionSystemId, NikeClassId, CommandSource::Player)));
	stepGalaxy(world);

	Yard* helionYard = yardAtSystem(world, HelionSystemId);
	REQUIRE(helionYard != nullptr);
	REQUIRE(helionYard->slips.front().hullClassId == NikeClassId);
	REQUIRE(helionYard->slips.front().remainingHullBuildTicks == nikeClass->buildTicks);
}

}
}
