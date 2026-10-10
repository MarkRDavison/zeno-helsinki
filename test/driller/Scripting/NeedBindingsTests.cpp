#include <catch2/catch_test_macros.hpp>
#include <Entities/Need.hpp>
#include <Scripting/NeedBindings.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <stdexcept>
#include <string>

namespace drl
{
namespace NeedBindingsTests
{

	std::string shipped(const char* relative)
	{
		return std::string(DRILLER_DATA_DIR) + "/" + relative;
	}

	TEST_CASE("shipped needs.lua registers sleep food and recreation", "[drl][Scripting]")
	{
		hl::scripting::LuaState lua;
		NeedPrototypeService needs;
		lua.runFile(shipped("Scripts/Base/needs.lua"));
		applyNeedsTable(lua.raw()["needs"], needs);

		REQUIRE(needs.registeredIds().size() == 3);

		const auto sleepId = needIdFromName("Need_Sleep");
		REQUIRE(needs.isPrototypeRegistered(sleepId));
		const auto& sleep = needs.getPrototype(sleepId);
		REQUIRE(sleep.name == "Need_Sleep");
		REQUIRE(sleep.decayPerSecond == 1.0f);
		REQUIRE(sleep.seekBelow == 25.0f);
		REQUIRE(sleep.collapseBelow == 0.0f);
		REQUIRE(sleep.priority == 0);
		REQUIRE(sleep.order == 1);
		REQUIRE(sleep.label == "Sleep");

		const auto foodId = needIdFromName("Need_Food");
		REQUIRE(needs.isPrototypeRegistered(foodId));
		const auto& food = needs.getPrototype(foodId);
		REQUIRE(food.name == "Need_Food");
		REQUIRE(food.seekBelow == 20.0f);
		REQUIRE(food.collapseBelow == 0.0f);
		REQUIRE(food.label == "Food");

		const auto recId = needIdFromName("Need_Recreation");
		REQUIRE(needs.isPrototypeRegistered(recId));
		const auto& rec = needs.getPrototype(recId);
		REQUIRE(rec.name == "Need_Recreation");
		REQUIRE(rec.seekBelow == 10.0f);
		REQUIRE(rec.collapseBelow == -1.0f);
		REQUIRE(rec.label == "Recreation");
	}

	TEST_CASE("missing needs global throws LuaError", "[drl][Scripting]")
	{
		hl::scripting::LuaState lua;
		NeedPrototypeService needs;
		lua.runString("x = 1");
		REQUIRE_THROWS_AS(applyNeedsTable(lua.raw()["needs"], needs), hl::scripting::LuaError);
	}

	TEST_CASE("need entry missing name throws LuaError", "[drl][Scripting]")
	{
		hl::scripting::LuaState lua;
		NeedPrototypeService needs;
		lua.runString(R"(needs = { { decayPerSecond = 1.0, seekBelow = 25, priority = 0, order = 1, label = "Sleep" } })");
		REQUIRE_THROWS_AS(applyNeedsTable(lua.raw()["needs"], needs), hl::scripting::LuaError);
	}

	TEST_CASE("unknown need prototype id throws", "[drl][NeedPrototypeService]")
	{
		NeedPrototypeService needs;
		REQUIRE_THROWS_AS(needs.getPrototype(needIdFromName("Need_DoesNotExist")), std::runtime_error);
	}

}
}
