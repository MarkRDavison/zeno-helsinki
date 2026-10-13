#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace ForbidCiviliansTests
{

void clearAvonSteel(WorldData& world)
{
	StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	for (auto& stock : avon->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 0.f;
			return;
		}
	}
}

TEST_CASE("forbidding Ashreach blocks unescorted Ashreach dest but not Helion steel", "[slr][ForbidCivilians]")
{
	WorldData world = makeStarterMap();
	clearAvonSteel(world);
	REQUIRE(execute(
		world,
		GameCommand::setCivilianHaulForbidden(
			HelionEmpireId,
			AshreachEmpireId,
			true,
			CommandSource::Player)));
	REQUIRE(civilianHaulIsForbidden(world, HelionEmpireId, AshreachEmpireId));
	REQUIRE(civilianLanePath(world, 5, 3, HelionEmpireId) == shortestLanePath(world, 5, 3));
	REQUIRE(civilianLanePath(world, 5, 1, HelionEmpireId) == shortestLanePath(world, 5, 1));
	REQUIRE(civilianLanePath(world, 3, 4, HelionEmpireId).empty());

	StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	rimward->stocks[0].goodAmount = TruckCapacity_IronOre;
	const HaulJob* steel = haulJobFor(world, 1, SteelGoodId);
	const HaulJob* ore = haulJobFor(world, 3, IronOreGoodId);
	REQUIRE(steel != nullptr);
	REQUIRE(ore != nullptr);
	REQUIRE_FALSE(execute(world, GameCommand::acceptHaulJob(1, steel->id, CommandSource::Ai)));
	REQUIRE(execute(world, GameCommand::acceptHaulJob(1, ore->id, CommandSource::Ai)));
}

TEST_CASE("clearing the forbid restores the Kestrel path", "[slr][ForbidCivilians]")
{
	WorldData world = makeStarterMap();
	REQUIRE(execute(
		world,
		GameCommand::setCivilianHaulForbidden(
			HelionEmpireId,
			AshreachEmpireId,
			true,
			CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::setCivilianHaulForbidden(
			HelionEmpireId,
			AshreachEmpireId,
			false,
			CommandSource::Player)));
	REQUIRE_FALSE(civilianHaulIsForbidden(world, HelionEmpireId, AshreachEmpireId));
	REQUIRE(civilianLanePath(world, 5, 1, HelionEmpireId) == shortestLanePath(world, 5, 1));
}

TEST_CASE("standing escort still takes Helion steel when Ashreach claims are forbidden", "[slr][ForbidCivilians]")
{
	WorldData world = makeStarterMap();
	world.trucks.erase(world.trucks.begin() + 1, world.trucks.end());
	world.convoys.clear();
	Truck& truck = world.trucks.front();
	truck.haulJobId = 0;
	truck.convoyId = 0;
	truck.cargoGoodId = 0;
	truck.cargoAmount = 0;
	parkTruckAt(truck, 5);
	StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	rimward->stocks.push_back(
		GoodStock{ .goodId = SteelGoodId, .goodAmount = TruckCapacity_Steel });

	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 5;
	home->motion.route.clear();
	home->motion.hop = 0;
	home->motion.progress = 0.0f;

	REQUIRE(execute(
		world,
		GameCommand::setCivilianHaulForbidden(
			HelionEmpireId,
			AshreachEmpireId,
			true,
			CommandSource::Player)));
	REQUIRE(execute(world, GameCommand::setFleetEscort(1, true, 1, 0, 0, CommandSource::Player)));
	stepGalaxy(world);

	const Convoy* standing = world.convoyById(home->escortConvoyId);
	REQUIRE(standing != nullptr);
	const HaulJob* steel = world.haulJobById(standing->haulJobId);
	REQUIRE(steel != nullptr);
	REQUIRE(steel->destinationSystemId == 1);
	REQUIRE(steel->goodId == SteelGoodId);
}

TEST_CASE("cannot forbid an empire's own claims", "[slr][ForbidCivilians]")
{
	WorldData world = makeStarterMap();
	REQUIRE_FALSE(execute(
		world,
		GameCommand::setCivilianHaulForbidden(
			HelionEmpireId,
			HelionEmpireId,
			true,
			CommandSource::Player)));
	REQUIRE(world.civilianHaulForbids.empty());
}

}
}
