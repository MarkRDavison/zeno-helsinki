#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace YardSlipTests
{

	long long hullCount(const WorldData& world)
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

TEST_CASE("Helion and Vesper start with one idle slip", "[slr][YardSlip]")
{
	const WorldData world = makeStarterMap();
	const Yard* helionYard = yardAtSystem(world, 1);
	const Yard* vesperYard = yardAtSystem(world, 8);
	REQUIRE(helionYard != nullptr);
	REQUIRE(vesperYard != nullptr);
	REQUIRE(helionYard->slips.size() == 1);
	REQUIRE(vesperYard->slips.size() == 1);
	REQUIRE(helionYard->slips.front().id == 1);
	REQUIRE(vesperYard->slips.front().id == 2);
	REQUIRE(countIdleYardSlips(*helionYard) == 1);
	REQUIRE(countIdleYardSlips(*vesperYard) == 1);
}

TEST_CASE("StartYardSlip fails when available steel is below cost", "[slr][YardSlip]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 250);
	REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 50);

	const bool started = execute(world, GameCommand::startYardSlip(1, CommandSource::Player));
	REQUIRE_FALSE(started);
	REQUIRE(yardAtSystem(world, 1)->slips.size() == 1);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 250);
}

TEST_CASE("StartYardSlip spends steel, builds one slip in eight ticks, and does not add hulls", "[slr][YardSlip]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 400);
	const long long hullsBefore = hullCount(world);

	REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 200);
	REQUIRE(execute(world, GameCommand::startYardSlip(1, CommandSource::Player)));
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 200);
	REQUIRE(stockpileAvailableForYardConsume(world, 1, SteelGoodId) == 0);
	REQUIRE(helion->stocks.front().deferredNeed == YardSlipSteelCost);

	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(helionYard != nullptr);
	REQUIRE(helionYard->slips.size() == 2);
	REQUIRE(countIdleYardSlips(*helionYard) == 1);
	const YardSlip* constructing = slipById(*helionYard, 3);
	REQUIRE(constructing != nullptr);
	REQUIRE(constructing->remainingBuildTicks == YardSlipBuildTicks);

	for (int tick = 0; tick < YardSlipBuildTicks; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(countIdleYardSlips(*helionYard) == 2);
	REQUIRE(slipById(*helionYard, 3)->remainingBuildTicks == 0);
	REQUIRE(hullCount(world) == hullsBefore);
}

TEST_CASE("two slips construct in parallel; one tick advances the lower id", "[slr][YardSlip]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 600);

	REQUIRE(execute(world, GameCommand::startYardSlip(1, CommandSource::Player)));
	REQUIRE(execute(world, GameCommand::startYardSlip(1, CommandSource::Player)));

	Yard* helionYard = yardAtSystem(world, 1);
	REQUIRE(helionYard != nullptr);
	REQUIRE(helionYard->slips.size() == 3);
	REQUIRE(slipById(*helionYard, 3)->remainingBuildTicks == YardSlipBuildTicks);
	REQUIRE(slipById(*helionYard, 4)->remainingBuildTicks == YardSlipBuildTicks);

	stepGalaxy(world);

	REQUIRE(slipById(*helionYard, 3)->remainingBuildTicks == YardSlipBuildTicks - 1);
	REQUIRE(slipById(*helionYard, 4)->remainingBuildTicks == YardSlipBuildTicks);

	for (int tick = 0; tick < 15; ++tick)
	{
		stepGalaxy(world);
	}

	REQUIRE(countIdleYardSlips(*helionYard) == 3);
	REQUIRE(slipById(*helionYard, 3)->remainingBuildTicks == 0);
	REQUIRE(slipById(*helionYard, 4)->remainingBuildTicks == 0);
}

TEST_CASE("useFromStockpile on StartYardSlip posts deferred need", "[slr][YardSlip]")
{
	WorldData world = makeStarterMap();
	silenceHelionTrafficAndPopulation(world);
	StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	setSteelPile(*helion, 400);

	REQUIRE(execute(world, GameCommand::startYardSlip(1, CommandSource::Player)));
	REQUIRE(helion->stocks.front().deferredNeed == YardSlipSteelCost);
	REQUIRE(helion->stocks.front().deferredNeedHeldOneTick == false);
	REQUIRE(getNeed(*helion, SteelGoodId) == 0);
}

}
}
