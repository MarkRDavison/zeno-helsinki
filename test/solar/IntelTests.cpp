#include <World/GameCommand.hpp>
#include <World/Intel.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <PutAtJump.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace IntelTests
{

	WorldData combatArena()
	{
		WorldData world;
		world.masterSeed = 42;
		world.empires = {
			Empire{ .id = 1, .name = "A" },
			Empire{ .id = 2, .name = "B" },
		};
		world.systems = {
			StarSystem{ .id = 10, .name = "Arena", .position = { 0.0f, 0.0f } },
		};
		world.engagementPolicies = {
			EngagementPolicy{
				.id = 1,
				.empireId = 1,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.encounterStance = EncounterStance::HoldPosition },
			EngagementPolicy{
				.id = 2,
				.empireId = 2,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.encounterStance = EncounterStance::HoldPosition },
		};
		world.empires[0].defaultPolicyId = 1;
		world.empires[1].defaultPolicyId = 2;
		world.shipClasses.push_back(ShipClass{
			.id = 1,
			.empireId = 1,
			.name = "Gun",
			.range = 200.0f,
			.accuracy = 1.0f,
			.laserDamage = 50,
			.maxShields = 0,
			.maxHull = 20,
			.magazineSize = 1 });
		world.shipClasses.push_back(ShipClass{
			.id = 2,
			.empireId = 2,
			.name = "Target",
			.range = 200.0f,
			.accuracy = 0.0f,
			.laserDamage = 5,
			.maxShields = 0,
			.maxHull = 8,
			.magazineSize = 1 });
		world.fleets.push_back(Fleet{
			.id = 1,
			.empireId = 1,
			.name = "A",
			.systemId = 10,
			.motion = { .speed = 50.0f } });
		appendFullMagazineHulls(world, world.fleets.back(), 1, 1);
		world.fleets.push_back(Fleet{
			.id = 2,
			.empireId = 2,
			.name = "B",
			.systemId = 10,
			.motion = { .speed = 50.0f } });
		appendFullMagazineHulls(world, world.fleets.back(), 2, 1);
		return world;
	}

	long long observationCountAtTick(const WorldData& world, long long observerEmpireId, long long tick)
	{
		long long count = 0;
		for (const auto& row : world.observedEnemyShipClasses)
		{
			if (row.observerEmpireId == observerEmpireId && row.firstObservedTick == tick)
			{
				++count;
			}
		}

		return count;
	}

	void setHoldPositionOnPolicy(WorldData& world, long long policyId, long long empireId)
	{
		const EngagementPolicy* policy = world.engagementPolicyById(policyId);
		REQUIRE(policy != nullptr);
		REQUIRE(execute(
			world,
			GameCommand::upsertEngagementPolicy(
				policyId,
				empireId,
				policy->name,
				policy->ruleOfEngagement,
				CommandSource::Player,
				policy->approach,
				EncounterStance::HoldPosition)));
	}

TEST_CASE("wiped empire gains no class intel from a battle", "[slr][Intel]")
{
	WorldData world = combatArena();
	const long long tickBefore = world.tick;
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(fleetShipCount(*world.fleetById(1)) == 1);
	REQUIRE(fleetShipCount(*world.fleetById(2)) == 0);
	REQUIRE(observationCountAtTick(world, 2, tickBefore) == 0);
	REQUIRE(!observerHasSeenEnemyClass(world, 2, 1, 1));
	REQUIRE(observerHasSeenEnemyClass(world, 1, 2, 2));
}

TEST_CASE("survivor records enemy ship class from before composition", "[slr][Intel]")
{
	WorldData world = combatArena();
	const long long tickBefore = world.tick;
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	const ObservedEnemyShipClass* row = observedEnemyShipClass(world, 1, 2, 2);
	REQUIRE(row != nullptr);
	REQUIRE(row->firstObservedTick == tickBefore);
	REQUIRE(row->sightingCount == 1);
	REQUIRE(world.battleReports.front().intelGainedThisBattle.size() == 1);
}

TEST_CASE("repeat battle increments sighting count without new first sighting", "[slr][Intel]")
{
	WorldData world = combatArena();
	stepGalaxy(world);
	const ObservedEnemyShipClass* row = observedEnemyShipClass(world, 1, 2, 2);
	REQUIRE(row != nullptr);
	const long long firstTick = row->firstObservedTick;
	Fleet* winner = world.fleetById(1);
	Fleet* loser = world.fleetById(2);
	REQUIRE(winner != nullptr);
	REQUIRE(loser != nullptr);
	appendFullMagazineHulls(world, *loser, 2, 1);
	loser->systemId = 10;
	loser->motion.route.clear();
	loser->motion.progress = 0.0f;
	winner->systemId = 10;
	winner->motion.route.clear();
	winner->motion.progress = 0.0f;
	const long long intelRowsBefore = static_cast<long long>(world.observedEnemyShipClasses.size());
	stepGalaxy(world);
	row = observedEnemyShipClass(world, 1, 2, 2);
	REQUIRE(row != nullptr);
	REQUIRE(row->firstObservedTick == firstTick);
	REQUIRE(row->sightingCount == 2);
	REQUIRE(static_cast<long long>(world.observedEnemyShipClasses.size()) == intelRowsBefore);
	REQUIRE(world.battleReports.back().intelGainedThisBattle.empty());
}

TEST_CASE("picket not in meeting is not on the battle report", "[slr][Intel]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	Fleet* kestrel = world.fleetById(3);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	REQUIRE(kestrel != nullptr);
	home->systemId = KestrelSystemId;
	home->placeId = KestrelFactoryPlaceId;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	rival->systemId = KestrelSystemId;
	rival->placeId = KestrelFactoryPlaceId;
	rival->motion.route.clear();
	rival->motion.progress = 0.0f;
	kestrel->systemId = KestrelSystemId;
	kestrel->motion.route.clear();
	kestrel->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 3, HelionSystemId);
	setHoldPositionOnPolicy(world, HelionWeaponsFreePolicyId, HelionEmpireId);
	setHoldPositionOnPolicy(world, AshreachWeaponsFreePolicyId, AshreachEmpireId);
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(1, HelionWeaponsFreePolicyId, CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(2, AshreachWeaponsFreePolicyId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	const BattleReport& report = world.battleReports.back();
	for (const auto& side : report.sides)
	{
		REQUIRE(side.fleetId != kestrel->id);
	}
}

TEST_CASE("survivor intel is deterministic for the same seed", "[slr][Intel]")
{
	WorldData a = combatArena();
	WorldData b = combatArena();
	stepGalaxy(a);
	stepGalaxy(b);
	REQUIRE(a.observedEnemyShipClasses.size() == b.observedEnemyShipClasses.size());
	for (size_t i = 0; i < a.observedEnemyShipClasses.size(); ++i)
	{
		REQUIRE(a.observedEnemyShipClasses[i].observerEmpireId ==
				b.observedEnemyShipClasses[i].observerEmpireId);
		REQUIRE(a.observedEnemyShipClasses[i].subjectEmpireId ==
				b.observedEnemyShipClasses[i].subjectEmpireId);
		REQUIRE(a.observedEnemyShipClasses[i].classId == b.observedEnemyShipClasses[i].classId);
		REQUIRE(a.observedEnemyShipClasses[i].firstObservedTick ==
				b.observedEnemyShipClasses[i].firstObservedTick);
		REQUIRE(a.observedEnemyShipClasses[i].sightingCount ==
				b.observedEnemyShipClasses[i].sightingCount);
	}
}

TEST_CASE("first sighting of a class only the enemy fields", "[slr][Intel]")
{
	WorldData world;
	world.masterSeed = 7;
	world.empires = {
		Empire{ .id = 1, .name = "A" },
		Empire{ .id = 2, .name = "B" },
	};
	world.systems = { StarSystem{ .id = 10, .name = "Arena", .position = { 0.0f, 0.0f } } };
	world.engagementPolicies = {
		EngagementPolicy{
			.id = 1,
			.empireId = 1,
			.name = "Weapons free",
			.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
			.encounterStance = EncounterStance::HoldPosition },
		EngagementPolicy{
			.id = 2,
			.empireId = 2,
			.name = "Weapons free",
			.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
			.encounterStance = EncounterStance::HoldPosition },
	};
	world.empires[0].defaultPolicyId = 1;
	world.empires[1].defaultPolicyId = 2;
	world.shipClasses.push_back(ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Common",
		.range = 200.0f,
		.accuracy = 0.5f,
		.laserDamage = 10,
		.maxShields = 0,
		.maxHull = 30,
		.magazineSize = 1 });
	world.shipClasses.push_back(ShipClass{
		.id = 99,
		.empireId = 2,
		.name = "Secret",
		.range = 200.0f,
		.accuracy = 0.5f,
		.laserDamage = 10,
		.maxShields = 0,
		.maxHull = 30,
		.magazineSize = 1 });
	world.fleets.push_back(Fleet{
		.id = 1,
		.empireId = 1,
		.name = "A",
		.systemId = 10,
		.motion = { .speed = 50.0f } });
	appendFullMagazineHulls(world, world.fleets.back(), 1, 2);
	world.fleets.push_back(Fleet{
		.id = 2,
		.empireId = 2,
		.name = "B",
		.systemId = 10,
		.motion = { .speed = 50.0f } });
	appendFullMagazineHulls(world, world.fleets.back(), 99, 2);
	REQUIRE(!observerHasSeenEnemyClass(world, 1, 2, 99));
	stepGalaxy(world);
	REQUIRE(observerHasSeenEnemyClass(world, 1, 2, 99));
	const ObservedEnemyShipClass* row = observedEnemyShipClass(world, 1, 2, 99);
	REQUIRE(row != nullptr);
	REQUIRE(row->sightingCount == 1);
}

}
}