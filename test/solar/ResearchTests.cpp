#include <World/GameCommand.hpp>
#include <World/Research.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

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

	TEST_CASE("idle wastes the empire research pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::stopResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) == Catch::Approx(0.f));
		REQUIRE(countActiveResearchProjects(world, HelionEmpireId) == 0);
	}

	TEST_CASE("one active project receives the full pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) ==
			Catch::Approx(EmpireResearchPerTick));
	}

	TEST_CASE("two active projects each receive a quarter of the pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectArmourId,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) == Catch::Approx(250.f));
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectArmourId) == Catch::Approx(250.f));
	}

	TEST_CASE("three active projects each receive one ninth of the pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectArmourId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectDriveId,
				CommandSource::Player)));
		stepGalaxy(world);
		const ResearchAmount ninth = EmpireResearchPerTick / 9.f;
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) == Catch::Approx(ninth));
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectArmourId) == Catch::Approx(ninth));
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectDriveId) == Catch::Approx(ninth));
	}

	TEST_CASE("stop keeps progress and restart continues it", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(execute(
			world,
			GameCommand::stopResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) ==
			Catch::Approx(EmpireResearchPerTick));
		const ResearchProgress* stopped =
			world.researchProgressByEmpireAndProject(HelionEmpireId, ResearchProjectSensorsId);
		REQUIRE(stopped != nullptr);
		REQUIRE_FALSE(stopped->active);
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) ==
			Catch::Approx(EmpireResearchPerTick));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) ==
			Catch::Approx(EmpireResearchPerTick * 2.f));
	}

	TEST_CASE("progress at cost completes and leaves the active set", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		ResearchProgress* row =
			world.researchProgressByEmpireAndProject(HelionEmpireId, ResearchProjectSensorsId);
		REQUIRE(row != nullptr);
		row->progress = ResearchProjectSensorsCost - 1.f;
		stepGalaxy(world);
		REQUIRE(row->completed);
		REQUIRE_FALSE(row->active);
		REQUIRE(row->progress == Catch::Approx(ResearchProjectSensorsCost));
		REQUIRE(countActiveResearchProjects(world, HelionEmpireId) == 0);
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
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
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(world.researchProgressByEmpireAndProject(HelionEmpireId, ResearchProjectSensorsId) ==
			nullptr);
	}

	TEST_CASE("empires do not share a research pool", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectArmourId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				AshreachEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		stepGalaxy(world);
		REQUIRE(progressOf(world, HelionEmpireId, ResearchProjectSensorsId) == Catch::Approx(250.f));
		REQUIRE(progressOf(world, AshreachEmpireId, ResearchProjectSensorsId) ==
			Catch::Approx(EmpireResearchPerTick));
	}

	TEST_CASE("starter map has no active research", "[slr][Research]")
	{
		const WorldData world = makeStarterMap();
		REQUIRE(countActiveResearchProjects(world, HelionEmpireId) == 0);
		REQUIRE(countActiveResearchProjects(world, AshreachEmpireId) == 0);
		REQUIRE(world.researchProgress.empty());
	}

	TEST_CASE("illegal research orders drop", "[slr][Research]")
	{
		WorldData world = twoEmpireWorld();
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(99, ResearchProjectSensorsId, CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(HelionEmpireId, 99, CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::stopResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
		REQUIRE_FALSE(execute(
			world,
			GameCommand::startResearchProject(
				HelionEmpireId,
				ResearchProjectSensorsId,
				CommandSource::Player)));
	}

	TEST_CASE("displayed research amounts floor partial values", "[slr][Research]")
	{
		REQUIRE(researchDisplayedAmount(1000.f / 9.f) == 111);
		REQUIRE(researchDisplayedAmount(4999.9f) == 4999);
	}

}

}
