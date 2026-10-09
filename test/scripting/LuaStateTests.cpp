#include <catch2/catch_test_macros.hpp>
#include <helsinki/Scripting/LuaState.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace hl::scripting::test
{
	namespace
	{
		class TempLuaFile
		{
		public:
			TempLuaFile(std::string_view fileName, std::string_view contents)
			{
				_path = std::filesystem::temp_directory_path() / std::string(fileName);
				std::ofstream out(_path, std::ios::binary | std::ios::trunc);
				out << contents;
			}

			~TempLuaFile()
			{
				std::error_code ec;
				std::filesystem::remove(_path, ec);
			}

			std::string path() const
			{
				return _path.string();
			}

		private:
			std::filesystem::path _path;
		};
	}

	TEST_CASE("runString sets a global", "[Scripting]")
	{
		LuaState lua;
		lua.runString("answer = 42");
		REQUIRE(lua.raw().get<int>("answer") == 42);
	}

	TEST_CASE("runString bad chunk throws LuaError", "[Scripting]")
	{
		LuaState lua;
		REQUIRE_THROWS_AS(lua.runString("this is not lua !!!", "chunk"), LuaError);
	}

	TEST_CASE("runFile loads an absolute path", "[Scripting]")
	{
		TempLuaFile file("helsinki_scripting_ok.lua", "flag = true");
		LuaState lua;
		lua.runFile(file.path());
		REQUIRE(lua.raw().get<bool>("flag"));
	}

	TEST_CASE("runFile missing file throws LuaError", "[Scripting]")
	{
		LuaState lua;
		const std::string missing = (std::filesystem::temp_directory_path() / "helsinki_scripting_missing.lua").string();
		REQUIRE_THROWS_AS(lua.runFile(missing), LuaError);
	}

	TEST_CASE("runFile syntax error throws LuaError with traceback", "[Scripting]")
	{
		TempLuaFile file("helsinki_scripting_bad.lua", "function (");
		LuaState lua;
		try
		{
			lua.runFile(file.path());
			FAIL("expected LuaError");
		}
		catch (const LuaError& err)
		{
			REQUIRE_FALSE(std::string(err.what()).empty());
			REQUIRE_FALSE(err.traceback().empty());
		}
	}

	TEST_CASE("callProtected success returns true", "[Scripting]")
	{
		LuaState lua;
		lua.runString("function ok() return 1 end");
		sol::protected_function fn = lua.raw()["ok"];
		REQUIRE(lua.callProtected(fn));
	}

	TEST_CASE("callProtected error does not throw", "[Scripting]")
	{
		LuaState lua;
		lua.runString("function boom() error('boom') end");
		sol::protected_function fn = lua.raw()["boom"];
		int gameData = 1;
		REQUIRE_NOTHROW(lua.callProtected(fn));
		REQUIRE_FALSE(lua.callProtected(fn));
		REQUIRE(gameData == 1);
	}
}
