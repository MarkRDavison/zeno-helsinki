#include <World/Combat.hpp>
#include <World/GameCommand.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace DoctrineTests
{

	constexpr long long ClosePolicyId = 1;
	constexpr long long HoldPolicyAId = 3;
	constexpr long long HoldPolicyBId = 4;

	WorldData laserBoatArena(FleetApproach approachA, FleetApproach approachB)
	{
		WorldData world;
		world.masterSeed = 1;
		world.empires = {
			Empire{ .id = 1, .name = "A", .defaultPolicyId = ClosePolicyId },
			Empire{ .id = 2, .name = "B", .defaultPolicyId = 2 },
		};
		world.systems = {
			StarSystem{ .id = 10, .name = "Arena", .position = { 0.0f, 0.0f } },
		};
		world.engagementPolicies = {
			EngagementPolicy{
				.id = ClosePolicyId,
				.empireId = 1,
				.name = "Close",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.approach = FleetApproach::CloseToLaserRange },
			EngagementPolicy{
				.id = 2,
				.empireId = 2,
				.name = "Close",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.approach = FleetApproach::CloseToLaserRange },
			EngagementPolicy{
				.id = HoldPolicyAId,
				.empireId = 1,
				.name = "Hold",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.approach = FleetApproach::HoldMissileRange },
			EngagementPolicy{
				.id = HoldPolicyBId,
				.empireId = 2,
				.name = "Hold",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.approach = FleetApproach::HoldMissileRange },
		};
		const ShipClass boat{
			.id = 1,
			.empireId = 1,
			.name = "Boat",
			.range = 40.0f,
			.accuracy = 1.0f,
			.laserDamage = 50,
			.maxShields = 0,
			.maxHull = 8,
			.closeRate = 10.0f,
			.hullRoles = hullRoleBit(HullRole::Warship),
			.missileTubes = 1,
			.missileReloadInnerTicks = 3,
			.magazineSize = 1,
			.missileDamage = 0,
			.missileRange = 90.0f,
			.missileSpeed = 25.0f };
		world.shipClasses.push_back(boat);
		ShipClass boatB = boat;
		boatB.id = 2;
		boatB.empireId = 2;
		world.shipClasses.push_back(boatB);
		world.fleets.push_back(Fleet{
			.id = 1,
			.empireId = 1,
			.name = "FA",
			.systemId = 10,
			.motion = { .speed = 50.0f },
			.policyId = approachA == FleetApproach::HoldMissileRange ? HoldPolicyAId : ClosePolicyId });
		appendFullMagazineHulls(world, world.fleets.back(), 1, 1);
		world.fleets.push_back(Fleet{
			.id = 2,
			.empireId = 2,
			.name = "FB",
			.systemId = 10,
			.motion = { .speed = 50.0f },
			.policyId = approachB == FleetApproach::HoldMissileRange ? HoldPolicyBId : 2 });
		appendFullMagazineHulls(world, world.fleets.back(), 2, 1);
		return world;
	}

TEST_CASE("upsert sets approach on a policy", "[slr][Doctrine]")
{
	WorldData world = makeStarterMap();
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			0,
			HelionEmpireId,
			"Kite",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::HoldMissileRange)));
	const EngagementPolicy* kite = nullptr;
	for (const auto& policy : world.engagementPolicies)
	{
		if (policy.name == "Kite" && policy.empireId == HelionEmpireId)
		{
			kite = &policy;
		}
	}

	REQUIRE(kite != nullptr);
	REQUIRE(kite->approach == FleetApproach::HoldMissileRange);
	REQUIRE(world.engagementPolicyById(HelionHoldRangePolicyId) != nullptr);
	REQUIRE(
		world.engagementPolicyById(HelionHoldRangePolicyId)->approach ==
		FleetApproach::HoldMissileRange);
}

TEST_CASE("both close still fights with accuracy 1", "[slr][Doctrine]")
{
	WorldData world =
		laserBoatArena(FleetApproach::CloseToLaserRange, FleetApproach::CloseToLaserRange);
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(fleetShipCount(*world.fleetById(1)) + fleetShipCount(*world.fleetById(2)) < 2);
}

TEST_CASE("hold versus close enters laser range and someone dies", "[slr][Doctrine]")
{
	WorldData world =
		laserBoatArena(FleetApproach::HoldMissileRange, FleetApproach::CloseToLaserRange);
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(fleetShipCount(*world.fleetById(1)) + fleetShipCount(*world.fleetById(2)) < 2);
}

TEST_CASE("both hold never enter laser range and both survive", "[slr][Doctrine]")
{
	WorldData world =
		laserBoatArena(FleetApproach::HoldMissileRange, FleetApproach::HoldMissileRange);
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(fleetShipCount(*world.fleetById(1)) == 1);
	REQUIRE(fleetShipCount(*world.fleetById(2)) == 1);
}

}
}
