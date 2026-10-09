#include <catch2/catch_test_macros.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Worker.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{
namespace GameCommandServiceTests
{

struct Fixture
	{
		TerrainData data;
		JobData jobData;
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
		JobPrototypeService prototypes;
		JobCreationService jobCreation{ jobData, prototypes, terrain };
		WorkerData workerData;
		WorkerPrototypeService workerPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes };
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		BuildingPlacementService buildings{ buildingData, terrain, recruitment, jobCreation, buildingPrototypes };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings };

		Fixture()
		{
			economy.setMax(ResourceOre, -1);
			economy.set(ResourceOre, 0);
			economy.setMax(ResourceMoney, -1);
			economy.set(ResourceMoney, 500);
		}
};

TEST_CASE("tick is monotonic from 0", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.currentTick() == 0);
	f.commands.tick();
	REQUIRE(f.commands.currentTick() == 1);
	f.commands.tick();
	REQUIRE(f.commands.currentTick() == 2);
}

TEST_CASE("player DigShaft skip-level does not pay", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE_FALSE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Player, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == -1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("player DigShaft cannot afford does not dig", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	f.economy.set(ResourceMoney, 50);
	REQUIRE_FALSE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Player, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 0);
	REQUIRE(f.economy.get(ResourceMoney) == 50);
}

TEST_CASE("player DigShaft pays 100 times level then digs", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Player, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 400);
}

TEST_CASE("setup DigShaft next level does not charge", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.execute(GameCommand::digShaft(0, CommandSource::Setup, CommandContext::DiggingShaft)));
	REQUIRE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Setup, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("system DigShaft next level does not charge", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.execute(GameCommand::digShaft(0, CommandSource::System, CommandContext::DiggingShaft)));
	REQUIRE(f.commands.execute(GameCommand::digShaft(1, CommandSource::System, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("DigTile succeeds and refuses non-contiguous on the right", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.commands.execute(GameCommand::digTile(0, 1, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE(f.terrain.isTileDugOut(0, 1));
	REQUIRE_FALSE(f.commands.execute(GameCommand::digTile(0, 3, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE_FALSE(f.terrain.doesTileExist(0, 3));
}

TEST_CASE("DigTile succeeds and refuses non-contiguous on the left", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.commands.execute(GameCommand::digTile(0, -1, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE(f.terrain.isTileDugOut(0, -1));
	REQUIRE_FALSE(f.commands.execute(GameCommand::digTile(0, -3, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE_FALSE(f.terrain.doesTileExist(0, -3));
}

TEST_CASE("AddResource increases ore and money", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.execute(GameCommand::addResource(ResourceOre, 12, CommandSource::System, CommandContext::AddResource)));
	REQUIRE(f.commands.execute(GameCommand::addResource(ResourceMoney, 25, CommandSource::System, CommandContext::AddResource)));
	REQUIRE(f.economy.get(ResourceOre) == 12);
	REQUIRE(f.economy.get(ResourceMoney) == 525);
}

TEST_CASE("CreateJob succeeds then reserved refuses", "[drl][GameCommandService]")
{
	Fixture f;
	JobPrototype prototype{};
	prototype.name = "Job_Dig";
	prototype.work = 1.0f;
	f.prototypes.registerPrototype(std::move(prototype));
	REQUIRE(f.terrain.digShaft(0));
	f.terrain.initialiseTile(0, 1);

	// CreateJobEvent.new("Job_Dig", "", level, column)
	REQUIRE(f.commands.execute(GameCommand::createJob(
		"Job_Dig",
		"",
		0,
		1,
		CommandSource::Player,
		CommandContext::CreatingJob)));
	REQUIRE(f.terrain.getTile(0, 1).jobReserved);
	REQUIRE(f.jobData.jobs.size() == 1);
	REQUIRE_FALSE(f.commands.execute(GameCommand::createJob(
		"Job_Dig",
		"",
		0,
		1,
		CommandSource::Player,
		CommandContext::CreatingJob)));
	REQUIRE(f.jobData.jobs.size() == 1);
}

TEST_CASE("CreateJob unknown prototype refuses", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	f.terrain.initialiseTile(0, 1);
	REQUIRE_FALSE(f.commands.execute(GameCommand::createJob(
		"Job_Dig",
		"",
		0,
		1,
		CommandSource::Player,
		CommandContext::CreatingJob)));
	REQUIRE(f.jobData.jobs.empty());
	REQUIRE_FALSE(f.terrain.getTile(0, 1).jobReserved);
}

TEST_CASE("CreateWorker succeeds at coordinates", "[drl][GameCommandService]")
{
	Fixture f;
	WorkerPrototype prototype{};
	prototype.name = "Worker_Builder";
	f.workerPrototypes.registerPrototype(std::move(prototype));
	REQUIRE(f.commands.execute(GameCommand::createWorker(
		"Worker_Builder",
		glm::vec2(1.0f, 0.0f),
		CommandSource::Setup,
		CommandContext::CreatingWorker)));
	REQUIRE(f.workerData.workers.size() == 1);
	REQUIRE(f.workerData.workers[0].position == glm::vec2(1.0f, 0.0f));
	REQUIRE(f.workerData.workers[0].state == WorkerState::Idle);
}

TEST_CASE("CreateWorker unknown prototype refuses", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE_FALSE(f.commands.execute(GameCommand::createWorker(
		"Worker_Builder",
		glm::vec2(1.0f, 0.0f),
		CommandSource::Setup,
		CommandContext::CreatingWorker)));
	REQUIRE(f.workerData.workers.empty());
}

TEST_CASE("setup PlaceBuilding bunk succeeds then unknown fails", "[drl][GameCommandService]")
{
	Fixture f;
	BuildingPrototype bunk{};
	bunk.name = "Building_Bunk";
	bunk.size = glm::ivec2(2, 1);
	f.buildingPrototypes.registerPrototype(std::move(bunk));
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.terrain.digTile(0, 1));
	REQUIRE(f.terrain.digTile(0, 2));

	REQUIRE(f.commands.execute(GameCommand::placeBuilding(
		"Building_Bunk",
		0,
		1,
		CommandSource::Setup,
		CommandContext::PlacingBuilding)));
	REQUIRE(f.buildingData.buildings.size() == 1);
	REQUIRE(f.buildingData.buildings[0].coordinates == glm::ivec2(1, 0));
	REQUIRE(f.terrain.getTile(0, 1).hasBuilding);
	REQUIRE(f.terrain.getTile(0, 2).hasBuilding);

	REQUIRE_FALSE(f.commands.execute(GameCommand::placeBuilding(
		"Building_Missing",
		0,
		3,
		CommandSource::Setup,
		CommandContext::PlacingBuilding)));
}

}
}
