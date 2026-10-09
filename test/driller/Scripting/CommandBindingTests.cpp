#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>

using drl::bindGameCommands;
using drl::EconomyResourceService;
using drl::GameCommandService;
using drl::ResourceMoney;
using drl::ResourceOre;
using drl::TerrainAlterationService;
using drl::TerrainData;
using hl::scripting::LuaError;
using hl::scripting::LuaState;

namespace
{
	struct Fixture
	{
		TerrainData data;
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
		GameCommandService commands{ terrain, economy };
		LuaState lua;

		Fixture()
		{
			economy.setMax(ResourceOre, -1);
			economy.set(ResourceOre, 0);
			economy.setMax(ResourceMoney, -1);
			economy.set(ResourceMoney, 500);
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

TEST_CASE("bad chunk throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(f.lua.runString("this is not lua"), LuaError);
}
