#include <catch2/catch_test_macros.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Worker.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerJobUpdateService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <string>

namespace drl
{
namespace JobDigOnCompleteTests
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
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation };
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
			applyPrototypesTable(lua.raw()["prototypes"], jobPrototypes, workerPrototypes);
		}
	};

	TEST_CASE("shipped Job_Dig onComplete issues System DigTile", "[drl][Scripting][Job_Dig]")
	{
		hl::scripting::LuaState lua;
		Fixture f(lua);

		REQUIRE(f.commands.execute(GameCommand::digShaft(0, CommandSource::Setup, CommandContext::DiggingShaft)));
		f.terrain.initialiseTile(0, 1);
		REQUIRE(f.terrain.doesTileExist(0, 1));
		REQUIRE_FALSE(f.terrain.isTileDugOut(0, 1));
		REQUIRE(f.commands.execute(GameCommand::createJob(
			"Job_Dig",
			"",
			0,
			1,
			CommandSource::Player,
			CommandContext::CreatingJob)));

		REQUIRE(f.jobData.jobs.size() == 1);
		JobInstance& job = f.jobData.jobs.front();
		REQUIRE(job.work == 2.0f);

		WorkerInstance& worker = f.workerData.workers.emplace_back();
		worker.id = 1;
		worker.allocatedJobId = job.id;
		worker.state = WorkerState::WorkingJob;
		job.allocatedWorkerId = worker.id;

		f.jobUpdate.update(2.0f);

		REQUIRE(f.terrain.isTileDugOut(0, 1));
		REQUIRE_FALSE(f.terrain.getTile(0, 1).jobReserved);
		REQUIRE(f.jobData.jobs.empty());
		REQUIRE(f.workerData.workers.front().state == WorkerState::Idle);
		REQUIRE(f.workerData.workers.front().allocatedJobId == 0);
	}

}
}
