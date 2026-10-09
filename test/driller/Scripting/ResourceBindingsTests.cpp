#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Scripting/ResourceBindings.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/UpgradeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <helsinki/System/glm.hpp>
#include <string>

namespace drl
{
namespace ResourceBindingsTests
{

constexpr const char* kResourcesChunk = R"(
		resources = {
			{ name = "Resource_Ore", max = -1, amount = 0 },
			{ name = "Resource_Money", max = -1, amount = 500 }
		}
	)";

	struct Fixture
	{
		hl::scripting::LuaState lua;
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
		ShuttleData shuttleData;
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttleCreation{ shuttleData, shuttlePrototypes };
		UpgradeData upgradeData;
		UpgradeService upgrades{ upgradeData };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings, shuttleCreation, upgrades };

		Fixture()
		{
			bindPrototypeUserTypes(lua.raw());
			bindGameCommands(lua.raw(), commands);
		}
	};

std::string shipped(const char* relative)
{
	return std::string(DRILLER_DATA_DIR) + "/" + relative;
}

TEST_CASE("resources table loads ore and money", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(kResourcesChunk);
	applyResourcesTable(f.lua.raw()["resources"], f.economy);
	REQUIRE(f.economy.get(ResourceOre) == 0);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
	REQUIRE(f.economy.getMax(ResourceOre) == -1);
	REQUIRE(f.economy.getMax(ResourceMoney) == -1);
}

TEST_CASE("missing resources global throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString("x = 1");
	REQUIRE_THROWS_AS(applyResourcesTable(f.lua.raw()["resources"], f.economy), hl::scripting::LuaError);
}

TEST_CASE("resource entry missing name throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(R"(resources = { { max = -1, amount = 0 } })");
	REQUIRE_THROWS_AS(applyResourcesTable(f.lua.raw()["resources"], f.economy), hl::scripting::LuaError);
}

TEST_CASE("runFile missing path throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(f.lua.runFile("Z:/definitely/missing/resources.lua"), hl::scripting::LuaError);
}

TEST_CASE("shipped resources and initializeCommands set up the cavern", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runFile(shipped("Scripts/Base/resources.lua"));
	applyResourcesTable(f.lua.raw()["resources"], f.economy);
	f.lua.runFile(shipped("Scripts/Base/prototypes.lua"));
	applyPrototypesTable(f.lua.raw()["prototypes"], f.prototypes, f.workerPrototypes, f.buildingPrototypes, f.shuttlePrototypes);
	f.lua.runFile(shipped("Scripts/Base/initializeCommands.lua"));

	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.terrain.isTileDugOut(0, 1));
	REQUIRE(f.terrain.isTileDugOut(1, 7));
	REQUIRE(f.economy.get(ResourceMoney) == 500);
	REQUIRE(f.economy.get(ResourceOre) == 0);
	REQUIRE(f.workerData.workers.size() == 1);
	REQUIRE(f.shuttleData.shuttles.size() == 1);
	REQUIRE(f.workerData.workers[0].position == glm::vec2(1.0f, 0.0f));
	REQUIRE(f.terrain.getTile(0, 1).hasBuilding);
	REQUIRE(f.terrain.getTile(0, 2).hasBuilding);
	REQUIRE(f.terrain.getTile(0, 3).hasBuilding);
	REQUIRE(f.terrain.getTile(0, 5).hasBuilding);
	REQUIRE(f.terrain.getTile(1, 5).hasBuilding);
	REQUIRE(f.terrain.getTile(1, 1).hasBuilding);
	REQUIRE(f.buildingData.buildings.size() == 5);
	REQUIRE(f.buildingData.buildings[0].coordinates == glm::ivec2(1, 0));
	REQUIRE(f.buildingData.buildings[1].coordinates == glm::ivec2(3, 0));
	REQUIRE(f.buildingData.buildings[2].coordinates == glm::ivec2(5, 0));
	REQUIRE(f.buildingData.buildings[3].coordinates == glm::ivec2(5, 1));
	REQUIRE(f.buildingData.buildings[4].coordinates == glm::ivec2(1, 1));
	REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 2);
	REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 2);
	REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Refiner") == 2);
	REQUIRE(f.jobData.jobs.size() == 4);
	int mineJobs = 0;
	int refineJobs = 0;
	for (const JobInstance& job : f.jobData.jobs)
	{
		if (job.prototypeId == jobPrototypeIdFromName("Job_Mine"))
		{
			++mineJobs;
		}
		if (job.prototypeId == jobPrototypeIdFromName("Job_Refine"))
		{
			++refineJobs;
		}
	}
	REQUIRE(mineJobs == 2);
	REQUIRE(refineJobs == 2);
}

}
}
