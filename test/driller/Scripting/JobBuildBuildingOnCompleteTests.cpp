#include <catch2/catch_test_macros.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Worker.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/UpgradeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerJobUpdateService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <string>

namespace drl
{
namespace JobBuildBuildingOnCompleteTests
{

	std::string shipped(const char* relative)
	{
		return std::string(DRILLER_DATA_DIR) + "/" + relative;
	}

	struct Fixture
	{
		TerrainData terrainData;
		JobData jobData;
		TerrainAlterationService terrain{ terrainData };
		EconomyResourceService economy;
		JobPrototypeService jobPrototypes;
		JobCreationService jobCreation{ jobData, jobPrototypes, terrain };
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
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings, buildingPrototypes, shuttleCreation, upgrades };
		WorkerJobUpdateService jobUpdate{ workerData, jobData, terrain, jobPrototypes };

		explicit Fixture(hl::scripting::LuaState& lua)
		{
			economy.setMax(ResourceOre, -1);
			economy.set(ResourceOre, 0);
			economy.setMax(ResourceMoney, -1);
			economy.set(ResourceMoney, 500);
			bindPrototypeUserTypes(lua.raw());
			bindGameCommands(lua.raw(), commands);
			lua.runFile(shipped("Scripts/Base/prototypes.lua"));
			applyPrototypesTable(lua.raw()["prototypes"], jobPrototypes, workerPrototypes, buildingPrototypes, shuttlePrototypes);
		}
	};

	TEST_CASE("shipped Job_Build_Building onComplete places bunk", "[drl][Scripting][Job_Build_Building]")
	{
		hl::scripting::LuaState lua;
		Fixture f(lua);

		REQUIRE(f.commands.execute(GameCommand::digShaft(0, CommandSource::Setup, CommandContext::DiggingShaft)));
		REQUIRE(f.commands.execute(GameCommand::digTile(0, 1, CommandSource::Setup, CommandContext::DiggingTile)));
		REQUIRE(f.commands.execute(GameCommand::digTile(0, 2, CommandSource::Setup, CommandContext::DiggingTile)));
		REQUIRE(f.commands.execute(GameCommand::createJob(
			"Job_Build_Building",
			"Building_Bunk",
			0,
			1,
			CommandSource::Player,
			CommandContext::PlacingBuilding)));

		REQUIRE(f.jobData.jobs.size() == 1);
		JobInstance& job = f.jobData.jobs.front();
		REQUIRE(job.work == 5.0f);
		REQUIRE(job.additionalPrototypeId == prototypeIdFromName("Building_Bunk"));

		WorkerInstance& worker = f.workerData.workers.emplace_back();
		worker.id = 1;
		worker.allocatedJobId = job.id;
		worker.state = WorkerState::WorkingJob;
		job.allocatedWorkerId = worker.id;

		f.jobUpdate.update(5.0f);

		REQUIRE(f.jobData.jobs.empty());
		REQUIRE(f.buildingData.buildings.size() == 1);
		REQUIRE(f.buildingData.buildings[0].coordinates == glm::ivec2(1, 0));
		REQUIRE(f.buildingData.buildings[0].prototypeId == prototypeIdFromName("Building_Bunk"));
		REQUIRE(f.terrain.getTile(0, 1).hasBuilding);
		REQUIRE(f.terrain.getTile(0, 2).hasBuilding);
		REQUIRE_FALSE(f.terrain.getTile(0, 1).jobReserved);
		REQUIRE(f.workerData.workers.front().state == WorkerState::Idle);
		REQUIRE(f.workerData.workers.front().allocatedJobId == 0);
	}

}
}
