#include <World/Combat.hpp>
#include <World/GameCommand.hpp>
#include <World/PickFleet.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
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

	void addClass(WorldData& world, const ShipClass& shipClass)
	{
		world.shipClasses.push_back(shipClass);
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
			.composition = { ClassCount{ .classId = classId, .count = count } },
			.motion = { .progress = progress, .speed = 50.0f } });
	}

TEST_CASE("starter map has two empires and rival classes at Kestrel", "[slr][Combat]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.empireById(HelionEmpireId) != nullptr);
	REQUIRE(world.empireById(AshreachEmpireId) != nullptr);
	const Fleet* home = fleetNamed(world, "Home Fleet");
	const Fleet* rival = fleetNamed(world, "Kestrel Squadron");
	REQUIRE(home != nullptr);
	REQUIRE(rival != nullptr);
	REQUIRE(home->empireId == HelionEmpireId);
	REQUIRE(rival->empireId == AshreachEmpireId);
	REQUIRE(rival->systemId == 2);
	const ShipClass* nike = classNamed(world, "Nike");
	const ShipClass* lance = classNamed(world, "Lance");
	REQUIRE(nike != nullptr);
	REQUIRE(lance != nullptr);
	REQUIRE(nike->empireId == HelionEmpireId);
	REQUIRE(lance->empireId == AshreachEmpireId);
	REQUIRE(nike->id != lance->id);
	REQUIRE(lance->range > nike->range);
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
	world.trucks.push_back(Truck{
		.id = 9,
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
		Fleet* home = world.fleetById(1);
		REQUIRE(home != nullptr);
		home->systemId = 2;
		home->motion.route.clear();
		home->motion.progress = 0.0f;
		REQUIRE(execute(
			world,
			GameCommand::upsertEngagementPolicy(
				0,
				HelionEmpireId,
				"Weapons free",
				RuleOfEngagement::FireIfEnemySeen,
				CommandSource::Player)));
		long long freeId = 0;
		for (const auto& policy : world.engagementPolicies)
		{
			if (policy.empireId == HelionEmpireId && policy.name == "Weapons free")
			{
				freeId = policy.id;
			}
		}

		REQUIRE(freeId != 0);
		REQUIRE(execute(
			world,
			GameCommand::setFleetPolicyOverride(1, freeId, CommandSource::Player)));
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

TEST_CASE("rival marker is mirrored and both fleets pick at Kestrel", "[slr][Combat]")
{
	WorldData world = makeStarterMap();
	Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	home->systemId = 2;
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

}
}
