#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Scripting/UpgradeBindings.hpp>
#include <Services/UpgradeService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <string>
#include <vector>

namespace drl
{
namespace UpgradeBindingsTests
{

std::string shipped(const char* relative)
{
	return std::string(DRILLER_DATA_DIR) + "/" + relative;
}

TEST_CASE("upgrades table registers HUD meta", "[drl][Scripting]")
{
	hl::scripting::LuaState lua;
	UpgradeData data;
	UpgradeService upgrades{ data };
	lua.runString(R"(
		upgrades = {
			{ name = "Upgrade_Refine", order = 3, label = "Ore multiplier", description = "bonus" }
		}
	)");
	applyUpgradesTable(lua.raw()["upgrades"], upgrades);
	REQUIRE(upgrades.registeredHudNames() == std::vector<std::string>{ UpgradeRefine });
	REQUIRE(upgrades.getOrder(UpgradeRefine) == 3);
	REQUIRE(upgrades.getLabel(UpgradeRefine) == "Ore multiplier");
	REQUIRE(upgrades.getDescription(UpgradeRefine) == "bonus");
}

TEST_CASE("upgrade HUD order follows order field not lua table order", "[drl][Scripting]")
{
	hl::scripting::LuaState lua;
	UpgradeData data;
	UpgradeService upgrades{ data };
	lua.runString(R"(
		upgrades = {
			{ name = "Upgrade_B", order = 2, label = "B", description = "b" },
			{ name = "Upgrade_A", order = 1, label = "A", description = "a" }
		}
	)");
	applyUpgradesTable(lua.raw()["upgrades"], upgrades);
	const auto names = upgrades.registeredHudNames();
	REQUIRE(names.size() == 2);
	REQUIRE(names[0] == "Upgrade_A");
	REQUIRE(names[1] == "Upgrade_B");
}

TEST_CASE("upgrade entry missing fields throws LuaError", "[drl][Scripting]")
{
	hl::scripting::LuaState lua;
	UpgradeData data;
	UpgradeService upgrades{ data };
	lua.runString(R"(upgrades = { { name = "Upgrade_Refine", label = "Ore multiplier", description = "bonus" } })");
	REQUIRE_THROWS_AS(applyUpgradesTable(lua.raw()["upgrades"], upgrades), hl::scripting::LuaError);
}

TEST_CASE("shipped upgrades.lua loads Upgrade_Refine", "[drl][Scripting]")
{
	hl::scripting::LuaState lua;
	UpgradeData data;
	UpgradeService upgrades{ data };
	lua.runFile(shipped("Scripts/Base/upgrades.lua"));
	applyUpgradesTable(lua.raw()["upgrades"], upgrades);
	REQUIRE(upgrades.hasHud(UpgradeRefine));
	REQUIRE(upgrades.getLabel(UpgradeRefine) == "Ore multiplier");
}

TEST_CASE("missing upgrades global throws LuaError", "[drl][Scripting]")
{
	hl::scripting::LuaState lua;
	UpgradeData data;
	UpgradeService upgrades{ data };
	lua.runString("x = 1");
	REQUIRE_THROWS_AS(applyUpgradesTable(lua.raw()["upgrades"], upgrades), hl::scripting::LuaError);
}

}
}
