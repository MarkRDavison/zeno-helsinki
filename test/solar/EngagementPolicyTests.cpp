#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace EngagementPolicyTests
{

TEST_CASE("starter empires default to hold fire", "[slr][EngagementPolicy]")
{
	const WorldData world = makeStarterMap();
	const Empire* helion = world.empireById(HelionEmpireId);
	const Empire* ashreach = world.empireById(AshreachEmpireId);
	REQUIRE(helion != nullptr);
	REQUIRE(ashreach != nullptr);
	const EngagementPolicy* helionDefault = world.engagementPolicyById(helion->defaultPolicyId);
	const EngagementPolicy* ashreachDefault = world.engagementPolicyById(ashreach->defaultPolicyId);
	REQUIRE(helionDefault != nullptr);
	REQUIRE(ashreachDefault != nullptr);
	REQUIRE(helionDefault->ruleOfEngagement == RuleOfEngagement::DoNotFireUnlessFiredUpon);
	REQUIRE(ashreachDefault->ruleOfEngagement == RuleOfEngagement::DoNotFireUnlessFiredUpon);
	REQUIRE(world.fleetById(1)->policyId == 0);
	REQUIRE(activeEngagementPolicy(world, *world.fleetById(1)) == helionDefault);
}

TEST_CASE("two hold-fire fleets sharing a system do not fight", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 2;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
}

TEST_CASE("fleet override weapons-free starts a fight against hold fire", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 2;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
}

TEST_CASE("upsert creates then edits a policy", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			0,
			HelionEmpireId,
			"Picket",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player)));
	long long picketId = 0;
	for (const auto& policy : world.engagementPolicies)
	{
		if (policy.name == "Picket")
		{
			picketId = policy.id;
		}
	}

	REQUIRE(picketId != 0);
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			picketId,
			HelionEmpireId,
			"Picket hold",
			RuleOfEngagement::DoNotFireUnlessFiredUpon,
			CommandSource::Player)));
	const EngagementPolicy* picket = world.engagementPolicyById(picketId);
	REQUIRE(picket != nullptr);
	REQUIRE(picket->name == "Picket hold");
	REQUIRE(picket->ruleOfEngagement == RuleOfEngagement::DoNotFireUnlessFiredUpon);
}

TEST_CASE("cannot override a fleet with another empire's policy", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	REQUIRE_FALSE(execute(
		world,
		GameCommand::setFleetPolicyOverride(1, AshreachHoldFirePolicyId, CommandSource::Player)));
	REQUIRE(world.fleetById(1)->policyId == 0);
}

TEST_CASE("starter seeds weapons-free policies without using them as default", "[slr][EngagementPolicy]")
{
	const WorldData world = makeStarterMap();
	const EngagementPolicy* helionFree = world.engagementPolicyById(HelionWeaponsFreePolicyId);
	const EngagementPolicy* ashreachFree = world.engagementPolicyById(AshreachWeaponsFreePolicyId);
	REQUIRE(helionFree != nullptr);
	REQUIRE(ashreachFree != nullptr);
	REQUIRE(helionFree->ruleOfEngagement == RuleOfEngagement::FireIfEnemySeen);
	REQUIRE(ashreachFree->ruleOfEngagement == RuleOfEngagement::FireIfEnemySeen);
	REQUIRE(world.empireById(HelionEmpireId)->defaultPolicyId == HelionHoldFirePolicyId);
}

TEST_CASE("setting Helion default to weapons free starts a fight at Kestrel", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 2;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	REQUIRE(execute(
		world,
		GameCommand::setEmpireDefaultPolicy(
			HelionEmpireId,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	REQUIRE(home->policyId == 0);
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
}

TEST_CASE("hold-fire fleet leaves an enemy truck at the system", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	world.trucks.push_back(Truck{
		.id = 99,
		.empireId = AshreachEmpireId,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .speed = 25.0f },
		.cargoGoodId = SteelGoodId,
		.cargoAmount = 40 });
	stepGalaxy(world);
	REQUIRE(world.truckById(99) != nullptr);
	REQUIRE(world.battleReports.empty());
}

TEST_CASE("weapons-free fleet destroys an enemy truck and lowers inbound", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	StarSystem* caldera = world.systemById(6);
	REQUIRE(caldera != nullptr);
	world.haulJobs.push_back(HaulJob{
		.id = 99,
		.destinationSystemId = 6,
		.empireId = AshreachEmpireId,
		.goodId = IronOreGoodId,
		.amount = 40 });
	world.trucks.push_back(Truck{
		.id = 99,
		.empireId = AshreachEmpireId,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .speed = 25.0f },
		.cargoGoodId = IronOreGoodId,
		.cargoAmount = 40,
		.haulJobId = 99 });
	caldera->stocks.push_back(GoodStock{ .goodId = IronOreGoodId, .inbound = 40 });
	REQUIRE(execute(
		world,
		GameCommand::setEmpireDefaultPolicy(
			HelionEmpireId,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.truckById(99) == nullptr);
	REQUIRE(getInbound(*caldera, IronOreGoodId) == 0);
	REQUIRE(world.battleReports.empty());
}

TEST_CASE("trucks alone do not start a battle", "[slr][EngagementPolicy]")
{
	WorldData world = makeStarterMap();
	world.fleets.clear();
	world.trucks.push_back(Truck{
		.id = 99,
		.empireId = AshreachEmpireId,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .speed = 25.0f } });
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(world.truckById(99) != nullptr);
}

}
}
