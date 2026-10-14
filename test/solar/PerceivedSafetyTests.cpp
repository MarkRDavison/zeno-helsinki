#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/TruckCargo.hpp>
#include <PutAtJump.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

namespace slr
{
namespace PerceivedSafetyTests
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

TEST_CASE("Junction is adjacent to Ashreach hostile presence for Helion", "[slr][PerceivedSafety]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(systemHasParkedHostileFleet(world, 4, HelionEmpireId));
	REQUIRE_FALSE(systemHasParkedHostileFleet(world, 3, HelionEmpireId));
	REQUIRE(systemIsAdjacentToParkedHostileFleet(world, 3, HelionEmpireId));
	REQUIRE_FALSE(systemHasParkedHostileFleet(world, 3, AshreachEmpireId));
}

TEST_CASE("Helion trucks still haul Rimward to Junction while the rival parks at Ashreach", "[slr][PerceivedSafety]")
{
	WorldData world = makeStarterMap();
	clearAvonSteel(world);
	const HaulJob* job = haulJobFor(world, 3, IronOreGoodId);
	REQUIRE(job != nullptr);
	const long long jobId = job->id;
	stepGalaxy(world);
	REQUIRE(world.trucks[0].haulJobId == jobId);
	stepUntilTruckHasGalaxyRoute(world, world.trucks[0].id);
	REQUIRE(world.trucks[0].motion.route == civilianLanePath(world, 5, 3, HelionEmpireId));
	REQUIRE(civilianLanePath(world, 5, 3, HelionEmpireId) == shortestLanePath(world, 5, 3));
}

TEST_CASE("civilians will not enter a system with a parked enemy fleet", "[slr][PerceivedSafety]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	Fleet* rival = world.fleetById(2);
	REQUIRE(rival != nullptr);
	rival->systemId = 3;
	rival->motion = {};
	world.trucks.push_back(Truck{
		.id = 10,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .speed = 25.0f },
		.classId = HelionCivilianTruckClassId });
	const HaulJob* ore = haulJobFor(world, 3, IronOreGoodId);
	REQUIRE(ore != nullptr);
	REQUIRE_FALSE(execute(world, GameCommand::acceptHaulJob(10, ore->id, CommandSource::Ai)));
	REQUIRE(civilianLanePath(world, 5, 3, HelionEmpireId).empty());
	REQUIRE(civilianLanePath(world, 5, 1, HelionEmpireId).empty());
}

TEST_CASE("civilians prefer a quiet path over one hop beside an enemy fleet", "[slr][PerceivedSafety]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "Hot", .position = { 100.0f, 0.0f } },
		StarSystem{ .id = 3, .name = "B", .position = { 200.0f, 0.0f } },
		StarSystem{ .id = 4, .name = "Quiet", .position = { 100.0f, 100.0f } },
		StarSystem{ .id = 5, .name = "Enemy", .position = { 100.0f, -100.0f } },
	};
	world.hyperlanes = {
		Hyperlane{ .a = 1, .b = 2 },
		Hyperlane{ .a = 2, .b = 3 },
		Hyperlane{ .a = 1, .b = 4 },
		Hyperlane{ .a = 3, .b = 4 },
		Hyperlane{ .a = 2, .b = 5 },
	};
	world.fleets.push_back(Fleet{
		.id = 1,
		.empireId = AshreachEmpireId,
		.name = "Raider",
		.systemId = 5,
		.composition = { ClassCount{ .classId = 1, .count = 1 } } });

	REQUIRE(civilianLanePath(world, 1, 3, HelionEmpireId) == (std::vector<long long>{ 1, 4, 3 }));
	REQUIRE(shortestLanePath(world, 1, 3) == (std::vector<long long>{ 1, 2, 3 }));
}

TEST_CASE("standing escort takes the short path beside an enemy fleet", "[slr][PerceivedSafety]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "Hot", .position = { 100.0f, 0.0f } },
		StarSystem{ .id = 3, .name = "B", .position = { 200.0f, 0.0f } },
		StarSystem{ .id = 4, .name = "Quiet", .position = { 100.0f, 100.0f } },
		StarSystem{ .id = 5, .name = "Enemy", .position = { 100.0f, -100.0f } },
	};
	world.hyperlanes = {
		Hyperlane{ .a = 1, .b = 2 },
		Hyperlane{ .a = 2, .b = 3 },
		Hyperlane{ .a = 1, .b = 4 },
		Hyperlane{ .a = 3, .b = 4 },
		Hyperlane{ .a = 2, .b = 5 },
	};
	world.fleets.push_back(Fleet{
		.id = 1,
		.empireId = AshreachEmpireId,
		.name = "Raider",
		.systemId = 5,
		.composition = { ClassCount{ .classId = 1, .count = 1 } } });

	REQUIRE(civilianLanePath(world, 1, 3, HelionEmpireId, true) == (std::vector<long long>{ 1, 2, 3 }));
}

TEST_CASE("escort still cannot enter a parked enemy fleet", "[slr][PerceivedSafety]")
{
	WorldData world = makeStarterMap();
	Fleet* rival = world.fleetById(2);
	REQUIRE(rival != nullptr);
	rival->systemId = 3;
	rival->motion = {};
	REQUIRE(civilianLanePath(world, 5, 3, HelionEmpireId, true).empty());
	REQUIRE(civilianEnterHopCost(world, 3, HelionEmpireId, true) < 0);
}

TEST_CASE("a fight heats the node then fades", "[slr][PerceivedSafety]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "B", .position = { 100.0f, 0.0f } },
	};
	world.hyperlanes = { Hyperlane{ .a = 1, .b = 2 } };
	world.battleReports.push_back(BattleReport{ .id = 1, .tick = 0, .systemId = 2 });
	world.tick = 0;
	REQUIRE(systemHasHazardHeat(world, 2, HelionEmpireId));
	REQUIRE(civilianEnterHopCost(world, 2, HelionEmpireId) == 2);
	world.tick = CivilianHazardFadeTicks;
	REQUIRE_FALSE(systemHasHazardHeat(world, 2, HelionEmpireId));
	REQUIRE(civilianEnterHopCost(world, 2, HelionEmpireId) == 1);
}

TEST_CASE("a destroyed Helion truck heats that node for Helion not Ashreach", "[slr][PerceivedSafety]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "B", .position = { 100.0f, 0.0f } },
	};
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 2 } } });
	REQUIRE(destroyTruckAndReduceInbound(world, 1));
	REQUIRE(systemHasHazardHeat(world, 2, HelionEmpireId));
	REQUIRE_FALSE(systemHasHazardHeat(world, 2, AshreachEmpireId));
	REQUIRE(civilianEnterHopCost(world, 2, HelionEmpireId) == 2);
	REQUIRE(civilianEnterHopCost(world, 2, AshreachEmpireId) == 1);
	world.tick = CivilianHazardFadeTicks;
	REQUIRE_FALSE(systemHasHazardHeat(world, 2, HelionEmpireId));
}

TEST_CASE("an overdue convoy heats remaining nodes until it is no longer underway", "[slr][PerceivedSafety]")
{
	WorldData world;
	world.tick = 5;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "B", .position = { 100.0f, 0.0f } },
		StarSystem{ .id = 3, .name = "C", .position = { 200.0f, 0.0f } },
	};
	world.hyperlanes = {
		Hyperlane{ .a = 1, .b = 2 },
		Hyperlane{ .a = 2, .b = 3 },
	};
	world.convoys.push_back(Convoy{
		.id = 1,
		.empireId = HelionEmpireId,
		.loadSystemId = 1,
		.motion = { .route = { 1, 2, 3 }, .hop = 0, .progress = 0.0f, .speed = 25.0f },
		.expectedArrivalTick = 4 });
	REQUIRE(systemHasHazardHeat(world, 1, HelionEmpireId));
	REQUIRE(systemHasHazardHeat(world, 2, HelionEmpireId));
	REQUIRE(systemHasHazardHeat(world, 3, HelionEmpireId));
	REQUIRE_FALSE(systemHasHazardHeat(world, 2, AshreachEmpireId));
	world.convoys.front().motion.route = { 3 };
	REQUIRE_FALSE(systemHasHazardHeat(world, 2, HelionEmpireId));
	REQUIRE_FALSE(systemHasHazardHeat(world, 3, HelionEmpireId));
}

TEST_CASE("starter empires are not at war and civilians still use Kestrel", "[slr][PerceivedSafety]")
{
	const WorldData world = makeStarterMap();
	REQUIRE_FALSE(empiresAreAtWar(world, HelionEmpireId, AshreachEmpireId));
	REQUIRE(civilianLanePath(world, 5, 1, HelionEmpireId) == shortestLanePath(world, 5, 1));
	REQUIRE_FALSE(systemClaimBlocksCivilian(world, 2, HelionEmpireId));
}

TEST_CASE("at war civilians will not enter rival-claimed systems", "[slr][PerceivedSafety]")
{
	WorldData world = makeStarterMap();
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	REQUIRE(empiresAreAtWar(world, AshreachEmpireId, HelionEmpireId));
	REQUIRE_FALSE(systemClaimBlocksCivilian(world, 2, HelionEmpireId));
	REQUIRE(systemClaimBlocksCivilian(world, 4, HelionEmpireId));
	REQUIRE(systemClaimBlocksCivilian(world, 8, HelionEmpireId));
	REQUIRE_FALSE(systemClaimBlocksCivilian(world, 3, HelionEmpireId));
	REQUIRE_FALSE(systemClaimBlocksCivilian(world, 6, HelionEmpireId));
	REQUIRE(civilianLanePath(world, 5, 1, HelionEmpireId) == shortestLanePath(world, 5, 1));
	REQUIRE(civilianLanePath(world, 5, 3, HelionEmpireId) == shortestLanePath(world, 5, 3));
	REQUIRE(civilianLanePath(world, 3, 8, HelionEmpireId).empty());
	REQUIRE(civilianLanePath(world, 3, 8, HelionEmpireId, true).empty());
}

TEST_CASE("a fleet fight sets the empire pair at war", "[slr][PerceivedSafety]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 4;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(1, HelionWeaponsFreePolicyId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(empiresAreAtWar(world, HelionEmpireId, AshreachEmpireId));
}

}
}
