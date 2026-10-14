#include <World/Combat.hpp>
#include <World/GameCommand.hpp>
#include <World/PickFleet.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <PutAtJump.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

namespace slr
{
namespace CombatTests
{

	const Fleet* fleetNamed(const WorldData& world, const char* name)
	{
		for (const auto& fleet : world.fleets)
		{
			if (fleet.name == name)
			{
				return &fleet;
			}
		}

		return nullptr;
	}

	const ShipClass* classNamed(const WorldData& world, const char* name)
	{
		for (const auto& shipClass : world.shipClasses)
		{
			if (shipClass.name == name)
			{
				return &shipClass;
			}
		}

		return nullptr;
	}

	WorldData combatFixture(long long empireA, long long empireB, long long systemId)
	{
		WorldData world;
		world.masterSeed = 1;
		world.empires = {
			Empire{ .id = empireA, .name = "A" },
			Empire{ .id = empireB, .name = "B" },
		};
		world.systems = {
			StarSystem{ .id = systemId, .name = "Arena", .position = { 0.0f, 0.0f } },
		};
		world.engagementPolicies = {
			EngagementPolicy{
				.id = 1,
				.empireId = empireA,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
			EngagementPolicy{
				.id = 2,
				.empireId = empireB,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
		};
		world.empires[0].defaultPolicyId = 1;
		world.empires[1].defaultPolicyId = 2;
		return world;
	}

	void addClass(WorldData& world, ShipClass shipClass)
	{
		if (shipClass.magazineSize <= 0)
		{
			shipClass.magazineSize = 1;
		}

		world.shipClasses.push_back(shipClass);
	}

	void emptyFleetMissileAmmo(Fleet& fleet)
	{
		for (auto& hull : fleet.hulls)
		{
			hull.magazine.clear();
		}
	}

	void addFleet(
		WorldData& world,
		long long id,
		long long empireId,
		long long systemId,
		long long classId,
		long long count,
		float progress)
	{
		world.fleets.push_back(Fleet{
			.id = id,
			.empireId = empireId,
			.name = "F" + std::to_string(id),
			.systemId = systemId,
			.motion = { .progress = progress, .speed = 50.0f } });
		appendFullMagazineHulls(world, world.fleets.back(), classId, count);
	}

	void parkHomeAtAshreachWithRival(WorldData& world)
	{
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->systemId = AshreachSystemId;
		home->motion.route.clear();
		home->motion.progress = 0.0f;
		putFleetAtJumpToward(world, 1, JunctionSystemId);
	}

TEST_CASE("starter map has two empires and rival classes at Ashreach", "[slr][Combat]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.empireById(HelionEmpireId) != nullptr);
	REQUIRE(world.empireById(AshreachEmpireId) != nullptr);
	const Fleet* home = fleetNamed(world, "Home Fleet");
	const Fleet* rival = fleetNamed(world, "Ashreach Fleet");
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	REQUIRE(home->empireId == HelionEmpireId);
	REQUIRE(rival->empireId == AshreachEmpireId);
	REQUIRE(rival->systemId == 4);
	const ShipClass* nike = classNamed(world, "Nike");
	const ShipClass* lance = classNamed(world, "Lance");
	REQUIRE(nike != nullptr);
	REQUIRE(lance != nullptr);
	REQUIRE(nike->empireId == HelionEmpireId);
	REQUIRE(lance->empireId == AshreachEmpireId);
	REQUIRE(nike->id != lance->id);
	REQUIRE(lance->range > nike->range);
	REQUIRE(nike->missileRange > nike->range);
	REQUIRE(lance->missileRange > lance->range);
	REQUIRE(home->hulls.size() == 20);
	const ShipClass* destroyer = classNamed(world, "Destroyer");
	REQUIRE(destroyer != nullptr);
	REQUIRE(fleetMissileAmmo(*home) == 8 * nike->magazineSize + 12 * destroyer->magazineSize);
}

TEST_CASE("hostile parked fleets fight once in one galaxy tick", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Gun",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 5,
		.maxShields = 0,
		.maxHull = 20,
		.closeRate = 0.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Target",
		.range = 200.0f,
		.accuracy = 0.0f,
		.laserDamage = 5,
		.maxShields = 0,
		.maxHull = 8,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);

	const long long tickBefore = world.tick;
	stepGalaxy(world);
	REQUIRE(world.tick == tickBefore + 1);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(world.battleReports.front().systemId == 10);
	REQUIRE(world.battleReports.front().tick == tickBefore);
	REQUIRE(classCountFor(world.fleets[1].composition, 2) == 0);
	REQUIRE(classCountFor(world.fleets[0].composition, 1) == 1);
}

TEST_CASE("same empire pair does not fight", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Gun",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 1, 10, 1, 1, 0.0f);
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(fleetShipCount(world.fleets[0]) == 1);
	REQUIRE(fleetShipCount(world.fleets[1]) == 1);
}

TEST_CASE("in-transit fleet does not fight", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	world.systems.push_back(StarSystem{ .id = 11, .name = "Other", .position = { 100.0f, 0.0f } });
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Gun",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Target",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.25f);
	world.fleets.front().motion.route = { 10, 11 };
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(fleetShipCount(world.fleets[0]) == 1);
	REQUIRE(fleetShipCount(world.fleets[1]) == 1);
}

TEST_CASE("trucks in the system do not fight or die", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Gun",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Target",
		.range = 200.0f,
		.accuracy = 0.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 4,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	world.engagementPolicies.push_back(EngagementPolicy{
		.id = 3,
		.empireId = 2,
		.name = "Hold fire",
		.ruleOfEngagement = RuleOfEngagement::DoNotFireUnlessFiredUpon });
	world.empires[1].defaultPolicyId = 3;
	world.trucks.push_back(Truck{
		.id = 9,
		.empireId = 1,
		.motion = { .route = { 10 }, .hop = 0, .progress = 0.0f, .speed = 25.0f },
		.cargoGoodId = 1,
		.cargoAmount = 40 });
	stepGalaxy(world);
	REQUIRE(world.trucks.size() == 1);
	REQUIRE(world.trucks.front().id == 9);
	REQUIRE(world.trucks.front().cargoAmount == 40);
	REQUIRE_FALSE(world.battleReports.empty());
}

TEST_CASE("longer range fires while the other is still closing", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Long",
		.range = 80.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Short",
		.range = 10.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 1.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	stepGalaxy(world);
	REQUIRE(classCountFor(world.fleets[0].composition, 1) == 1);
	REQUIRE(classCountFor(world.fleets[1].composition, 2) == 0);
}

TEST_CASE("shields absorb then leftover hull; hull zero dies", "[slr][Combat]")
{
	long long shields = 10;
	long long hull = 8;
	applyLaserDamage(shields, hull, 4);
	REQUIRE(shields == 6);
	REQUIRE(hull == 8);

	applyLaserDamage(shields, hull, 6);
	REQUIRE(shields == 0);
	REQUIRE(hull == 8);

	applyLaserDamage(shields, hull, 8);
	REQUIRE(shields == 0);
	REQUIRE(hull == 0);
}

TEST_CASE("same seed twice matches folded counts", "[slr][Combat]")
{
	auto makeWorld = []()
	{
		WorldData world = makeStarterMap();
		parkHomeAtAshreachWithRival(world);
		REQUIRE(execute(
			world,
			GameCommand::setFleetPolicyOverride(
				1,
				HelionWeaponsFreePolicyId,
				CommandSource::Player)));
		return world;
	};

	WorldData a = makeWorld();
	WorldData b = makeWorld();
	stepGalaxy(a);
	stepGalaxy(b);
	REQUIRE(a.battleReports.size() == 1);
	REQUIRE(b.battleReports.size() == 1);
	REQUIRE(a.fleets[0].composition.size() == b.fleets[0].composition.size());
	REQUIRE(a.fleets[1].composition.size() == b.fleets[1].composition.size());
	for (size_t i = 0; i < a.fleets[0].composition.size(); ++i)
	{
		REQUIRE(a.fleets[0].composition[i].classId == b.fleets[0].composition[i].classId);
		REQUIRE(a.fleets[0].composition[i].count == b.fleets[0].composition[i].count);
	}
	for (size_t i = 0; i < a.fleets[1].composition.size(); ++i)
	{
		REQUIRE(a.fleets[1].composition[i].classId == b.fleets[1].composition[i].classId);
		REQUIRE(a.fleets[1].composition[i].count == b.fleets[1].composition[i].count);
	}
}

TEST_CASE("wipe leaves empty composition and report for both sides", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Gun",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Target",
		.range = 200.0f,
		.accuracy = 0.0f,
		.laserDamage = 1,
		.maxShields = 0,
		.maxHull = 1,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(world.battleReports.front().sides.size() == 2);
	REQUIRE(fleetShipCount(*world.fleetById(2)) == 0);
	REQUIRE(world.fleetById(2) != nullptr);
	REQUIRE(world.battleReports.front().sides[1].before.size() == 1);
	REQUIRE(world.battleReports.front().sides[1].after.empty());
}

TEST_CASE("Home Fleet arriving at Kestrel does not fight under hold fire", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	putFleetAtJumpToward(world, 1, 2);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(1, 2, CommandSource::Player)));
	REQUIRE(world.battleReports.empty());

	bool arrived = false;
	for (int i = 0; i < 64; ++i)
	{
		stepGalaxy(world);
		const Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		if (home->systemId == 2 && home->motion.route.empty())
		{
			arrived = true;
			break;
		}
	}

	REQUIRE(arrived);
	REQUIRE(world.battleReports.empty());
}

TEST_CASE("rival marker is mirrored and both fleets pick at Ashreach", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 4;
	const Fleet* rival = world.fleetById(2);
	REQUIRE(rival != nullptr);
	const auto homeMarker = fleetMarkerPosition(world, *home);
	const auto rivalMarker = fleetMarkerPosition(world, *rival);
	REQUIRE(homeMarker.has_value());
	REQUIRE(rivalMarker.has_value());
	REQUIRE(*homeMarker != *rivalMarker);
	REQUIRE(pickFleet(world, *homeMarker, GalaxyFleetSize * 0.5f) == home->id);
	REQUIRE(pickFleet(world, *rivalMarker, GalaxyFleetSize * 0.5f) == rival->id);
}

TEST_CASE("a missile salvo spends tubes and ammo stays down on the next fight", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Tubes",
		.range = 0.0f,
		.accuracy = 1.0f,
		.laserDamage = 0,
		.maxShields = 0,
		.maxHull = 100,
		.closeRate = 0.0f,
		.missileTubes = 2,
		.missileReloadInnerTicks = 10000,
		.magazineSize = 8,
		.missileDamage = 0,
		.missileRange = 200.0f,
		.missileSpeed = 25.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "TubesB",
		.range = 0.0f,
		.accuracy = 1.0f,
		.laserDamage = 0,
		.maxShields = 0,
		.maxHull = 100,
		.closeRate = 0.0f,
		.missileTubes = 2,
		.missileReloadInnerTicks = 10000,
		.magazineSize = 8,
		.missileDamage = 0,
		.missileRange = 200.0f,
		.missileSpeed = 25.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	REQUIRE(fleetMissileAmmo(world.fleets[0]) == 8);
	stepGalaxy(world);
	REQUIRE(fleetMissileAmmo(world.fleets[0]) == 6);
	REQUIRE(world.battleReports.front().sides[0].missilesBefore == 8);
	REQUIRE(world.battleReports.front().sides[0].missilesAfter == 6);
	REQUIRE(classCountFor(world.fleets[0].composition, 1) == 1);
	stepGalaxy(world);
	REQUIRE(fleetMissileAmmo(world.fleets[0]) == 4);
}

TEST_CASE("a dry hull still fires lasers", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Gun",
		.range = 200.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 20,
		.closeRate = 0.0f,
		.missileTubes = 2,
		.missileReloadInnerTicks = 3,
		.magazineSize = 8,
		.missileDamage = 50,
		.missileRange = 400.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Target",
		.range = 200.0f,
		.accuracy = 0.0f,
		.laserDamage = 5,
		.maxShields = 0,
		.maxHull = 8,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	world.fleets[0].hulls.front().magazine.clear();
	stepGalaxy(world);
	REQUIRE(classCountFor(world.fleets[1].composition, 2) == 0);
	REQUIRE(classCountFor(world.fleets[0].composition, 1) == 1);
	REQUIRE(fleetMissileAmmo(world.fleets[0]) == 0);
}

TEST_CASE("missiles reach farther than lasers", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "MissileBoat",
		.range = 10.0f,
		.accuracy = 1.0f,
		.laserDamage = 0,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f,
		.missileTubes = 1,
		.missileReloadInnerTicks = 1,
		.magazineSize = 4,
		.missileDamage = 50,
		.missileRange = 200.0f,
		.missileSpeed = 50.0f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "LaserOnly",
		.range = 10.0f,
		.accuracy = 1.0f,
		.laserDamage = 50,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	stepGalaxy(world);
	REQUIRE(classCountFor(world.fleets[0].composition, 1) == 1);
	REQUIRE(classCountFor(world.fleets[1].composition, 2) == 0);
	REQUIRE(fleetMissileAmmo(world.fleets[0]) < 4);
}

TEST_CASE("slow missiles spend ammo before they arrive", "[slr][Combat]")
{
	WorldData world = combatFixture(1, 2, 10);
	addClass(world, ShipClass{
		.id = 1,
		.empireId = 1,
		.name = "Slow",
		.range = 0.0f,
		.accuracy = 1.0f,
		.laserDamage = 0,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f,
		.missileTubes = 1,
		.missileReloadInnerTicks = 10000,
		.magazineSize = 4,
		.missileDamage = 50,
		.missileRange = 200.0f,
		.missileSpeed = 0.001f });
	addClass(world, ShipClass{
		.id = 2,
		.empireId = 2,
		.name = "Still",
		.range = 0.0f,
		.accuracy = 0.0f,
		.laserDamage = 0,
		.maxShields = 0,
		.maxHull = 10,
		.closeRate = 0.0f });
	addFleet(world, 1, 1, 10, 1, 1, 0.0f);
	addFleet(world, 2, 2, 10, 2, 1, 0.0f);
	stepGalaxy(world);
	REQUIRE(classCountFor(world.fleets[0].composition, 1) == 1);
	REQUIRE(classCountFor(world.fleets[1].composition, 2) == 1);
	REQUIRE(fleetMissileAmmo(world.fleets[0]) == 3);
}

TEST_CASE("two dry weapons-free fleets sit and do not fight", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	parkHomeAtAshreachWithRival(world);
	emptyFleetMissileAmmo(*home);
	emptyFleetMissileAmmo(*rival);
	const long long homeShips = fleetShipCount(*home);
	const long long rivalShips = fleetShipCount(*rival);
	REQUIRE(execute(
		world,
		GameCommand::setEmpireDefaultPolicy(
			HelionEmpireId,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::setEmpireDefaultPolicy(
			AshreachEmpireId,
			AshreachWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(home->systemId == 4);
	REQUIRE(rival->systemId == 4);
	REQUIRE(fleetShipCount(*home) == homeShips);
	REQUIRE(fleetShipCount(*rival) == rivalShips);
}

TEST_CASE("dry weapons-free does not open fire on a loaded hold-fire fleet", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	parkHomeAtAshreachWithRival(world);
	emptyFleetMissileAmmo(*home);
	REQUIRE(fleetHasJoinedMissileAmmo(*rival));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(home->systemId == 4);
	REQUIRE(rival->systemId == 4);
}

TEST_CASE("loaded weapons-free still fights a dry fleet in the same system", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	parkHomeAtAshreachWithRival(world);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			HelionHoldFirePolicyId,
			HelionEmpireId,
			"Hold fire",
			RuleOfEngagement::DoNotFireUnlessFiredUpon,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::HoldPosition)));
	emptyFleetMissileAmmo(*home);
	const long long rivalAmmoBefore = fleetMissileAmmo(*rival);
	REQUIRE(rivalAmmoBefore > 0);
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			HelionWeaponsFreePolicyId,
			HelionEmpireId,
			"Weapons free",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::HoldPosition)));
	REQUIRE(execute(
		world,
		GameCommand::setEmpireDefaultPolicy(
			HelionEmpireId,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			AshreachWeaponsFreePolicyId,
			AshreachEmpireId,
			"Weapons free",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::HoldPosition)));
	REQUIRE(execute(
		world,
		GameCommand::setEmpireDefaultPolicy(
			AshreachEmpireId,
			AshreachWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(home->systemId == 4);
	REQUIRE(rival->systemId == 4);
	REQUIRE(fleetMissileAmmo(*rival) < rivalAmmoBefore);
}

TEST_CASE("one remaining missile is enough to open fire", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	parkHomeAtAshreachWithRival(world);
	emptyFleetMissileAmmo(*home);
	REQUIRE(!home->hulls.empty());
	addMagazineGood(home->hulls.front().magazine, MissileGoodId, 1.f);
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
}

TEST_CASE("weapons-free fleets at two Kestrel jumps do not fight", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	home->systemId = KestrelSystemId;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 1, HelionSystemId);
	rival->systemId = KestrelSystemId;
	rival->motion.route.clear();
	rival->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 2, JunctionSystemId);
	const long long homeShips = fleetShipCount(*home);
	const long long rivalShips = fleetShipCount(*rival);
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(1, HelionWeaponsFreePolicyId, CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(2, AshreachWeaponsFreePolicyId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(fleetShipCount(*home) == homeShips);
	REQUIRE(fleetShipCount(*rival) == rivalShips);
}

TEST_CASE("weapons-free fleets at the Kestrel mill fight; a far jump picket survives", "[slr][Combat]")
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
	const long long picketShips = fleetShipCount(*kestrel);
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(1, HelionWeaponsFreePolicyId, CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(2, AshreachWeaponsFreePolicyId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
	REQUIRE(world.battleReports.front().systemId == KestrelSystemId);
	REQUIRE(fleetShipCount(*kestrel) == picketShips);
	REQUIRE(kestrel->placeId == jumpPlaceForLaneNeighbor(world, KestrelSystemId, HelionSystemId)->id);
}

TEST_CASE("flee stance dry fleet backs off from loaded enemy at same place", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	home->systemId = AshreachSystemId;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 1, VesperSystemId);
	rival->systemId = AshreachSystemId;
	rival->motion.route.clear();
	rival->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 2, VesperSystemId);
	emptyFleetMissileAmmo(*home);
	REQUIRE(fleetHasJoinedMissileAmmo(*rival));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			2,
			AshreachWeaponsFreePolicyId,
			CommandSource::Player)));
	const SystemPlace* junctionJump =
		jumpPlaceForLaneNeighbor(world, AshreachSystemId, JunctionSystemId);
	REQUIRE(junctionJump != nullptr);
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(inSystemMotionIsCrawling(home->inSystem));
	REQUIRE(home->inSystem.toPlaceId == junctionJump->id);
}

TEST_CASE("chase stance crawls toward distant hostile in the same system", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	home->systemId = HelionSystemId;
	home->placeId = HelionYardPlaceId;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	rival->systemId = HelionSystemId;
	rival->motion.route.clear();
	rival->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 2, KestrelSystemId);
	const SystemPlace* kestrelJump = jumpPlaceForLaneNeighbor(world, HelionSystemId, KestrelSystemId);
	REQUIRE(kestrelJump != nullptr);
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			HelionWeaponsFreePolicyId,
			HelionEmpireId,
			"Weapons free",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::Chase)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.empty());
	REQUIRE(inSystemMotionIsCrawling(home->inSystem));
	REQUIRE(home->inSystem.toPlaceId == kestrelJump->id);
}

TEST_CASE("chase does not follow a hostile onto a hyperlane", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	home->systemId = HelionSystemId;
	home->placeId = HelionYardPlaceId;
	home->motion.route.clear();
	home->motion.progress = 0.0f;
	rival->systemId = HelionSystemId;
	rival->motion.route.clear();
	rival->motion.progress = 0.0f;
	putFleetAtJumpToward(world, 2, KestrelSystemId);
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			HelionWeaponsFreePolicyId,
			HelionEmpireId,
			"Weapons free",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::Chase)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(inSystemMotionIsCrawling(home->inSystem));
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(2, KestrelSystemId, CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(home->motion.route.empty());
	REQUIRE(home->motion.progress == 0.0f);
}

TEST_CASE("chase at meeting range still resolves battle when roe allows", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	Fleet* rival = world.fleetById(2);
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	parkHomeAtAshreachWithRival(world);
	declareWarBetweenEmpires(world, HelionEmpireId, AshreachEmpireId);
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			HelionWeaponsFreePolicyId,
			HelionEmpireId,
			"Weapons free",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::Chase)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			1,
			HelionWeaponsFreePolicyId,
			CommandSource::Player)));
	REQUIRE(execute(
		world,
		GameCommand::upsertEngagementPolicy(
			AshreachWeaponsFreePolicyId,
			AshreachEmpireId,
			"Weapons free",
			RuleOfEngagement::FireIfEnemySeen,
			CommandSource::Player,
			FleetApproach::CloseToLaserRange,
			EncounterStance::HoldPosition)));
	REQUIRE(execute(
		world,
		GameCommand::setFleetPolicyOverride(
			2,
			AshreachWeaponsFreePolicyId,
			CommandSource::Player)));
	stepGalaxy(world);
	REQUIRE(world.battleReports.size() == 1);
}

}
}
