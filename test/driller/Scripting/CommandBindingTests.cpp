#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <helsinki/System/glm.hpp>

using drl::bindGameCommands;
using drl::bindPrototypeUserTypes;
using drl::EconomyResourceService;
using drl::GameCommandService;
using drl::JobCreationService;
using drl::JobData;
using drl::JobPrototype;
using drl::JobPrototypeService;
using drl::ResourceMoney;
using drl::ResourceOre;
using drl::TerrainAlterationService;
using drl::TerrainData;
using drl::WorkerCreationService;
using drl::WorkerData;
using drl::WorkerPrototype;
using drl::WorkerPrototypeService;
using hl::scripting::LuaError;
using hl::scripting::LuaState;

namespace
{
	struct Fixture
	{
		LuaState lua;
		TerrainData data;
		JobData jobData;
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
		JobPrototypeService prototypes;
		JobCreationService jobCreation{ jobData, prototypes, terrain };
		WorkerData workerData;
		WorkerPrototypeService workerPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation };

		Fixture()
		{
			economy.setMax(ResourceOre, -1);
			economy.set(ResourceOre, 0);
			economy.setMax(ResourceMoney, -1);
			economy.set(ResourceMoney, 500);
			bindPrototypeUserTypes(lua.raw());
			bindGameCommands(lua.raw(), commands);
		}
	};
}

TEST_CASE("cmd Setup DigShaft does not charge", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(R"(
		cmd(GameCommand.new(DigShaftEvent.new(0), GameCommandContext.DiggingShaft, GameCommandSource.Setup))
		cmd(GameCommand.new(DigShaftEvent.new(1), GameCommandContext.DiggingShaft, GameCommandSource.Setup))
	)");
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("cmd Player DigShaft refuses when broke", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(R"(
		cmd(GameCommand.new(DigShaftEvent.new(0), GameCommandContext.DiggingShaft, GameCommandSource.Setup))
	)");
	f.economy.set(ResourceMoney, 50);
	f.lua.runString(R"(
		cmd(GameCommand.new(DigShaftEvent.new(1), GameCommandContext.DiggingShaft, GameCommandSource.Player))
	)");
	REQUIRE(f.data.shaftLevel == 0);
	REQUIRE(f.economy.get(ResourceMoney) == 50);
}

TEST_CASE("cmd DigTile succeeds on left and right", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(R"(
		cmd(GameCommand.new(DigShaftEvent.new(0), GameCommandContext.DiggingShaft, GameCommandSource.Setup))
		cmd(GameCommand.new(DigTileEvent.new(0, 1), GameCommandContext.DiggingTile, GameCommandSource.Setup))
		cmd(GameCommand.new(DigTileEvent.new(0, -1), GameCommandContext.DiggingTile, GameCommandSource.Setup))
	)");
	REQUIRE(f.terrain.isTileDugOut(0, 1));
	REQUIRE(f.terrain.isTileDugOut(0, -1));
}

TEST_CASE("cmd AddResourceEvent adds ore and money", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(R"(
		cmd(GameCommand.new(AddResourceEvent.new("Resource_Ore", 12), GameCommandContext.AddResource, GameCommandSource.System))
		cmd(GameCommand.new(AddResourceEvent.new("Resource_Money", 25), GameCommandContext.AddResource, GameCommandSource.System))
	)");
	REQUIRE(f.economy.get(ResourceOre) == 12);
	REQUIRE(f.economy.get(ResourceMoney) == 525);
}

TEST_CASE("cmd CreateJobEvent uses level then column", "[drl][Scripting]")
{
	Fixture f;
	JobPrototype prototype{};
	prototype.name = "Job_Dig";
	prototype.work = 1.0f;
	f.prototypes.registerPrototype(std::move(prototype));
	f.lua.runString(R"(
		cmd(GameCommand.new(DigShaftEvent.new(0), GameCommandContext.DiggingShaft, GameCommandSource.Setup))
	)");
	f.terrain.initialiseTile(0, 1);
	f.lua.runString(R"(
		cmd(GameCommand.new(CreateJobEvent.new("Job_Dig", "", 0, 1), GameCommandContext.CreatingJob, GameCommandSource.Player))
	)");
	REQUIRE(f.jobData.jobs.size() == 1);
	REQUIRE(f.jobData.jobs[0].tile == glm::ivec2(1, 0));
	REQUIRE(f.terrain.getTile(0, 1).jobReserved);
}

TEST_CASE("cmd CreateWorkerEvent uses vec2f coordinates", "[drl][Scripting]")
{
	Fixture f;
	WorkerPrototype prototype{};
	prototype.name = "Worker_Builder";
	f.workerPrototypes.registerPrototype(std::move(prototype));
	f.lua.runString(R"(
		cmd(GameCommand.new(CreateWorkerEvent.new("Worker_Builder", vec2f.new(1.0, 0.0)), GameCommandContext.CreatingWorker, GameCommandSource.Setup))
	)");
	REQUIRE(f.workerData.workers.size() == 1);
	REQUIRE(f.workerData.workers[0].position == glm::vec2(1.0f, 0.0f));
}

TEST_CASE("bad chunk throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(f.lua.runString("this is not lua"), LuaError);
}
