#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
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
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/UpgradeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerJobUpdateService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <helsinki/System/glm.hpp>
#include <string>

namespace drl
{
namespace JobRefineOnCompleteTests
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
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes, buildingData, buildingPrototypes, needPrototypes };
		BuildingPlacementService buildings{ buildingData, terrain, recruitment, jobCreation, buildingPrototypes };
		ShuttleData shuttleData;
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttleCreation{ shuttleData, shuttlePrototypes };
		UpgradeData upgradeData;
		UpgradeService upgrades{ upgradeData };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings, buildingPrototypes, shuttleCreation, upgrades, workerData };
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
			applyPrototypesTable(lua.raw()["prototypes"], jobPrototypes, workerPrototypes, buildingPrototypes, shuttlePrototypes, needPrototypes);
			BuildingInstance housing{};
			housing.prototypeId = prototypeIdFromName("Building_Bunk");
			buildingData.buildings.push_back(housing);
		}

		void digFootprint(int startColumn, int width)
		{
			REQUIRE(commands.execute(GameCommand::digShaft(0, CommandSource::Setup, CommandContext::DiggingShaft)));
			if (startColumn > 0)
			{
				for (int x = 1; x <= startColumn + width - 1; ++x)
				{
					REQUIRE(commands.execute(GameCommand::digTile(0, x, CommandSource::Setup, CommandContext::DiggingTile)));
				}
			}
			else
			{
				for (int x = -1; x >= startColumn; --x)
				{
					REQUIRE(commands.execute(GameCommand::digTile(0, x, CommandSource::Setup, CommandContext::DiggingTile)));
				}
			}
		}
	};

	TEST_CASE("shipped Job_Refine onComplete stores upgrade and does not add ore", "[drl][Scripting][Job_Refine]")
	{
		for (const int startColumn : { 1, -4 })
		{
			hl::scripting::LuaState lua;
			Fixture f(lua);
			f.digFootprint(startColumn, 4);
			REQUIRE(f.commands.execute(GameCommand::placeBuilding(
				"Building_Refining",
				0,
				startColumn,
				CommandSource::Setup,
				CommandContext::PlacingBuilding)));
			REQUIRE(f.jobData.jobs.size() == 2);
			JobInstance& job = f.jobData.jobs.front();
			REQUIRE(job.prototypeId == jobPrototypeIdFromName("Job_Refine"));
			REQUIRE(job.work == 32.0f);

			REQUIRE(f.commands.execute(GameCommand::createWorker(
				"Worker_Refiner",
				glm::vec2(static_cast<float>(startColumn), 0.0f),
				CommandSource::Setup,
				CommandContext::CreatingWorker)));
			WorkerInstance& worker = f.workerData.workers.back();
			worker.allocatedJobId = job.id;
			worker.state = WorkerState::WorkingJob;
			job.allocatedWorkerId = worker.id;

			f.jobUpdate.update(32.0f);
			REQUIRE_THAT(f.upgrades.oreMultiplier(), Catch::Matchers::WithinAbs(1.001f, 0.0001f));
			REQUIRE(f.economy.get(ResourceOre) == 0);
			REQUIRE(f.jobData.jobs.size() == 2);
			REQUIRE(job.work == 32.0f);
			REQUIRE(worker.allocatedJobId == job.id);
			REQUIRE(job.allocatedWorkerId == worker.id);
			REQUIRE_FALSE(job.requiresRemoval);

			f.jobUpdate.update(32.0f);
			REQUIRE_THAT(f.upgrades.oreMultiplier(), Catch::Matchers::WithinAbs(1.002f, 0.0001f));
			REQUIRE(f.economy.get(ResourceOre) == 0);
			REQUIRE(job.work == 32.0f);
			REQUIRE(worker.state == WorkerState::WorkingJob);
		}
	}

}
}
