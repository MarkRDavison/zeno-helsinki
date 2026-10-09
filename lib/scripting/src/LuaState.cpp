#include <helsinki/Scripting/LuaState.hpp>

#include <filesystem>
#include <string>

namespace hl::scripting
{
	LuaState::LuaState()
	{
		_state.open_libraries();
	}

	sol::state& LuaState::raw()
	{
		return _state;
	}

	void LuaState::runString(std::string_view chunk, std::string_view chunkName)
	{
		sol::load_result loaded = _state.load(std::string(chunk), std::string(chunkName));
		if (!loaded.valid())
		{
			sol::error err = loaded;
			throwLuaError(err.what());
		}

		sol::protected_function_result result = loaded();
		if (!result.valid())
		{
			sol::error err = result;
			throwLuaError(err.what());
		}
	}

	void LuaState::runFile(const std::string& absolutePath)
	{
		if (!std::filesystem::exists(absolutePath))
		{
			throw LuaError("cannot open " + absolutePath + ": No such file");
		}

		sol::load_result loaded = _state.load_file(absolutePath);
		if (!loaded.valid())
		{
			sol::error err = loaded;
			throwLuaError(err.what());
		}

		sol::protected_function_result result = loaded();
		if (!result.valid())
		{
			sol::error err = result;
			throwLuaError(err.what());
		}
	}

	void LuaState::throwLuaError(std::string what)
	{
		throw LuaError(what, what);
	}
}
