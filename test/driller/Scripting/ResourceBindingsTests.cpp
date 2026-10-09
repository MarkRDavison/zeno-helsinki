#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Scripting/ResourceBindings.hpp>
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
#include <string>

using drl::applyPrototypesTable;
using drl::applyResourcesTable;
using drl::bindGameCommands;
using drl::bindPrototypeUserTypes;
using drl::EconomyResourceService;
using drl::GameCommandService;
using drl::JobCreationService;
using drl::JobData;
using drl::JobPrototypeService;
using drl::ResourceMoney;
using drl::ResourceOre;
using drl::TerrainAlterationService;
using drl::TerrainData;
using drl::WorkerCreationService;
using drl::WorkerData;
using drl::WorkerPrototypeService;
using hl::scripting::LuaError;
using hl::scripting::LuaState;

namespace
{
	constexpr const char* kResourcesChunk = R"(
		resources = {
			{ name = "Resource_Ore", max = -1, amount = 0 },
			{ name = "Resource_Money", max = -1, amount = 500 }
		}
	)";

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
			bindPrototypeUserTypes(lua.raw());
			bindGameCommands(lua.raw(), commands);
		}
	};

	std::string shipped(const char* relative)
	{
		return std::string(DRILLER_DATA_DIR) + "/" + relative;
	}
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
	REQUIRE_THROWS_AS(applyResourcesTable(f.lua.raw()["resources"], f.economy), LuaError);
}

TEST_CASE("resource entry missing name throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString(R"(resources = { { max = -1, amount = 0 } })");
	REQUIRE_THROWS_AS(applyResourcesTable(f.lua.raw()["resources"], f.economy), LuaError);
}

TEST_CASE("runFile missing path throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(f.lua.runFile("Z:/definitely/missing/resources.lua"), LuaError);
}

TEST_CASE("shipped resources and initializeCommands set up the cavern", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runFile(shipped("Scripts/Base/resources.lua"));
	applyResourcesTable(f.lua.raw()["resources"], f.economy);
	f.lua.runFile(shipped("Scripts/Base/prototypes.lua"));
	applyPrototypesTable(f.lua.raw()["prototypes"], f.prototypes, f.workerPrototypes);
	f.lua.runFile(shipped("Scripts/Base/initializeCommands.lua"));

	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.terrain.isTileDugOut(0, 1));
	REQUIRE(f.terrain.isTileDugOut(1, 7));
	REQUIRE(f.economy.get(ResourceMoney) == 500);
	REQUIRE(f.workerData.workers.size() == 1);
	REQUIRE(f.workerData.workers[0].position == glm::vec2(1.0f, 0.0f));
}
