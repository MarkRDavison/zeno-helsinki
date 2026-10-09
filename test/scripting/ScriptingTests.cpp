#include <catch2/catch_test_macros.hpp>
#include <helsinki/Scripting/Scripting.hpp>

#include <lua.h>

namespace hl::scripting::test
{
	TEST_CASE("helsinki-scripting links Lua 5.4", "[Scripting]")
	{
		REQUIRE(LUA_VERSION_NUM == 504);
	}
}
