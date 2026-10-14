#include <World/GameCommand.hpp>
#include <World/Research.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

namespace slr
{
namespace ResearchTests
{

	WorldData twoEmpireWorld()
	{
		WorldData world;
		world.empires = {
			Empire{ .id = HelionEmpireId, .name = "Helion" },
			Empire{ .id = AshreachEmpireId, .name = "Ashreach" },
		};
		return world;
	}

	ResearchAmount progressOf(const WorldData& world, long long empireId, long long projectId)
	{
		const ResearchProgress* row = world.researchProgressByEmpireAndProject(empireId, projectId);
		if (row == nullptr)
		{
			return 0.f;
		}

		return row->progress;
	}

	void completeProject(WorldData& world, long long empireId, long long projectId)
	{
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(empireId, projectId, CommandSource::Player)));
		ResearchProgress* row = world.researchProgressByEmpireAndProject(empireId, projectId);
		REQUIRE(row != nullptr);
		const ResearchProjectSpec* spec = researchProjectById(projectId);
		REQUIRE(spec != nullptr);
		row->progress = spec->cost - 1.f;
		stepGalaxy(world);
		REQUIRE(row->completed);
	}

	WorldData laserDuelWorld()
	{
		WorldData world;
		world.masterSeed = 1;
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
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
			EngagementPolicy{
				.id = 2,
				.empireId = 2,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
		};
		world.empires[0].defaultPolicyId = 1;
		world.empires[1].defaultPolicyId = 2;
		return world;
	}

	void addLaserBoat(
		WorldData& world,
		long long classId,
		long long empireId,
		float range,
		long long laserDamage,
		long long maxHull,
		float accuracy)
	{
		world.shipClasses.push_back(ShipClass{
			.id = classId,
			.empireId = empireId,
			.name = "Boat" + std::to_string(classId),
			.range = range,
			.accuracy = accuracy,
			.laserDamage = laserDamage,
			.maxShields = 0,
			.maxHull = maxHull,
			.closeRate = 0.0f,
			.magazineSize = 1 });
	}

	void addParkedFleet(
		WorldData& world,
		long long fleetId,
		long long empireId,
		long long classId)
	{
		world.fleets.push_back(Fleet{
			.id = fleetId,
			.empireId = empireId,
			.name = "F" + std::to_string(fleetId),
			.systemId = 10,
			.motion = { .speed = 50.0f } });
		appendFullMagazineHulls(world, world.fleets.back(), classId, 1);
	}

	TEST_CASE("idle wastes the empire research pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::stopResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) == Catch::Approx(0.f));
		REQUIRE(countActiveResearchProjects(world, HelionEmpireId) == 0);
	}

	TEST_CASE("one active project receives the full pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) ==
			Catch::Approx(EmpireResearchPerTick));
	}

	TEST_CASE("damage and range tracks can run together and split the pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserRange1Id,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) == Catch::Approx(250.f));
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserRange1Id) == Catch::Approx(250.f));
	}

	TEST_CASE("three active projects each receive one ninth of the pool", "[slr][Research]")
	{
		REQUIRE(researchPointsAddedPerActiveProject(3) == Catch::Approx(EmpireResearchPerTick / 9.f));
	}

	TEST_CASE("stop keeps progress and restart continues it", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(execute(
			world,
			GameCommand::stopResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) ==
			Catch::Approx(EmpireResearchPerTick));
		const ResearchProgress* stopped =
			world.researchProgressByEmpireAndProject(HelionEmpireId, ResearchProjectLaserDamage1Id);
		REQUIRE(stopped != nullptr);
		REQUIRE_FALSE(stopped->active);
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) ==
			Catch::Approx(EmpireResearchPerTick));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) ==
			Catch::Approx(EmpireResearchPerTick * 2.f));
	}

	TEST_CASE("progress at cost completes and leaves the active set", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		ResearchProgress* row =
			world.researchProgressByEmpireAndProject(HelionEmpireId, ResearchProjectLaserDamage1Id);
		REQUIRE(row != nullptr);
		row->progress = ResearchProjectLevel1Cost - 1.f;
		stepGalaxy(world);
		REQUIRE(row->completed);
		REQUIRE_FALSE(row->active);
		REQUIRE(row->progress == Catch::Approx(ResearchProjectLevel1Cost));
		REQUIRE(row->unlockApplied);
		REQUIRE(countActiveResearchProjects(world, HelionEmpireId) == 0);
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier ==
			Catch::Approx(LaserMultiplierLevel1));
	}

	TEST_CASE("start drops when the active cap is full", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		for (long long i = 0; i < MaxActiveResearchProjects; ++i)
		{
			world.researchProgress.push_back(ResearchProgress{
				.empireId = HelionEmpireId,
				.projectId = 100 + i,
				.progress = 0.f,
				.active = true,
				.completed = false });
		}

		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(world.researchProgressByEmpireAndProject(HelionEmpireId, ResearchProjectLaserDamage1Id) ==
			nullptr);
	}

	TEST_CASE("empires do not share a research pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserRange1Id,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				AshreachEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectLaserDamage1Id) == Catch::Approx(250.f));
		REQUIRE(progressOf(world, AshreachEmpireId, ResearchProjectLaserDamage1Id) ==
			Catch::Approx(EmpireResearchPerTick));
	}

	TEST_CASE("starter map has no active research and default laser stats", "[slr][Research]")
	{
		const WorldData world = makeStarterMap();
		REQUIRE(countActiveResearchProjects(world, HelionEmpireId) == 0);
		REQUIRE(countActiveResearchProjects(world, AshreachEmpireId) == 0);
		REQUIRE(world.researchProgress.empty());
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier == Catch::Approx(1.f));
		REQUIRE(empireStats(world, HelionEmpireId).laserRangeMultiplier == Catch::Approx(1.f));
	}

	TEST_CASE("illegal research orders drop", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				99, ResearchProjectLaserDamage1Id, CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(HelionEmpireId, 99, CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::stopResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
	}

	TEST_CASE("later track levels are gated on the previous", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage2Id,
				CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserRange2Id,
				CommandSource::Player)));
		completeProject(world, HelionEmpireId, ResearchProjectLaserDamage1Id);
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage2Id,
				CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		completeProject(world, HelionEmpireId, ResearchProjectLaserRange1Id);
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserRange2Id,
				CommandSource::Player)));
	}

	TEST_CASE("applyCompletedResearch sets level multipliers and is idempotent", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		world.researchProgress.push_back(ResearchProgress{
			.empireId = HelionEmpireId,
			.projectId = ResearchProjectLaserDamage1Id,
			.progress = ResearchProjectLevel1Cost,
			.completed = true });
		applyCompletedResearch(world, HelionEmpireId, ResearchProjectLaserDamage1Id);
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier ==
			Catch::Approx(LaserMultiplierLevel1));
		REQUIRE(empireStats(world, HelionEmpireId).laserRangeMultiplier == Catch::Approx(1.f));
		applyCompletedResearch(world, HelionEmpireId, ResearchProjectLaserDamage1Id);
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier ==
			Catch::Approx(LaserMultiplierLevel1));

		world.researchProgress.push_back(ResearchProgress{
			.empireId = HelionEmpireId,
			.projectId = ResearchProjectLaserDamage2Id,
			.progress = ResearchProjectLevel2Cost,
			.completed = true });
		applyCompletedResearch(world, HelionEmpireId, ResearchProjectLaserDamage2Id);
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier ==
			Catch::Approx(LaserMultiplierLevel2));

		world.researchProgress.push_back(ResearchProgress{
			.empireId = HelionEmpireId,
			.projectId = ResearchProjectLaserDamage3Id,
			.progress = ResearchProjectLevel3Cost,
			.completed = true });
		applyCompletedResearch(world, HelionEmpireId, ResearchProjectLaserDamage3Id);
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier ==
			Catch::Approx(LaserMultiplierLevel3));

		world.researchProgress.push_back(ResearchProgress{
			.empireId = HelionEmpireId,
			.projectId = ResearchProjectLaserRange1Id,
			.progress = ResearchProjectLevel1Cost,
			.completed = true });
		applyCompletedResearch(world, HelionEmpireId, ResearchProjectLaserRange1Id);
		REQUIRE(empireStats(world, HelionEmpireId).laserRangeMultiplier ==
			Catch::Approx(LaserMultiplierLevel1));
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier ==
			Catch::Approx(LaserMultiplierLevel3));
	}

	TEST_CASE("applyCompletedResearch no-ops when the project is not complete", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectLaserDamage1Id,
				CommandSource::Player)));
		applyCompletedResearch(world, HelionEmpireId, ResearchProjectLaserDamage1Id);
		REQUIRE(empireStats(world, HelionEmpireId).laserDamageMultiplier == Catch::Approx(1.f));
	}

	TEST_CASE("displayed research amounts floor partial values", "[slr][Research]")
	{
		REQUIRE(researchDisplayedAmount(1000.f / 9.f) == 111);
		REQUIRE(researchDisplayedAmount(4999.9f) == 4999);
	}

	TEST_CASE("laser damage research flips an even laser duel", "[slr][Research][Combat]")
	{
		WorldData unresearched = laserDuelWorld();
		addLaserBoat(unresearched, 1, 1, 200.0f, 4, 10, 1.0f);
		addLaserBoat(unresearched, 2, 2, 200.0f, 4, 10, 1.0f);
		addParkedFleet(unresearched, 1, 2, 2);
		addParkedFleet(unresearched, 2, 1, 1);
		stepGalaxy(unresearched);
		REQUIRE(classCountFor(unresearched.fleetById(2)->composition, 1) == 0);
		REQUIRE(classCountFor(unresearched.fleetById(1)->composition, 2) == 1);

		WorldData researched = laserDuelWorld();
		addLaserBoat(researched, 1, 1, 200.0f, 4, 10, 1.0f);
		addLaserBoat(researched, 2, 2, 200.0f, 4, 10, 1.0f);
		addParkedFleet(researched, 1, 2, 2);
		addParkedFleet(researched, 2, 1, 1);
		researched.researchProgress.push_back(ResearchProgress{
			.empireId = 1,
			.projectId = ResearchProjectLaserDamage1Id,
			.progress = ResearchProjectLevel1Cost,
			.completed = true });
		applyCompletedResearch(researched, 1, ResearchProjectLaserDamage1Id);
		stepGalaxy(researched);
		REQUIRE(classCountFor(researched.fleetById(2)->composition, 1) == 1);
		REQUIRE(classCountFor(researched.fleetById(1)->composition, 2) == 0);
	}

	TEST_CASE("laser range research lets a short-ranged ship fire", "[slr][Research][Combat]")
	{
		WorldData unresearched = laserDuelWorld();
		addLaserBoat(unresearched, 1, 1, 90.0f, 50, 20, 1.0f);
		addLaserBoat(unresearched, 2, 2, 200.0f, 50, 8, 0.0f);
		addParkedFleet(unresearched, 1, 1, 1);
		addParkedFleet(unresearched, 2, 2, 2);
		stepGalaxy(unresearched);
		REQUIRE(classCountFor(unresearched.fleetById(1)->composition, 1) == 1);
		REQUIRE(classCountFor(unresearched.fleetById(2)->composition, 2) == 1);

		WorldData researched = laserDuelWorld();
		addLaserBoat(researched, 1, 1, 90.0f, 50, 20, 1.0f);
		addLaserBoat(researched, 2, 2, 200.0f, 50, 8, 0.0f);
		addParkedFleet(researched, 1, 1, 1);
		addParkedFleet(researched, 2, 2, 2);
		researched.researchProgress.push_back(ResearchProgress{
			.empireId = 1,
			.projectId = ResearchProjectLaserRange1Id,
			.progress = ResearchProjectLevel1Cost,
			.completed = true });
		applyCompletedResearch(researched, 1, ResearchProjectLaserRange1Id);
		stepGalaxy(researched);
		REQUIRE(classCountFor(researched.fleetById(1)->composition, 1) == 1);
		REQUIRE(classCountFor(researched.fleetById(2)->composition, 2) == 0);
	}

}

}
