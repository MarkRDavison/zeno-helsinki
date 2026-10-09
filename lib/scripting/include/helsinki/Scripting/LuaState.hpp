#pragma once

#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/System/Utils/NonCopyable.hpp>
#include <sol/sol.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace hl::scripting
{
	class LuaState : public hl::NonCopyable
	{
	public:
		LuaState();

		sol::state& raw();

		void runFile(const std::string& absolutePath);
		void runString(std::string_view chunk, std::string_view chunkName = "chunk");

		template<typename... Args>
		bool callProtected(const sol::protected_function& fn, Args&&... args);

	private:
		[[noreturn]] static void throwLuaError(std::string what);
		static void logError(std::string_view message);

		sol::state _state;
	};

	template<typename... Args>
	bool LuaState::callProtected(const sol::protected_function& fn, Args&&... args)
	{
		if (!fn.valid())
		{
			logError("protected function is invalid");
			return false;
		}

		sol::protected_function_result result = fn(std::forward<Args>(args)...);
		if (!result.valid())
		{
			sol::error err = result;
			logError(err.what());
			return false;
		}

		return true;
	}

	inline void LuaState::logError(std::string_view message)
	{
		std::cerr << "[hl::scripting] " << message << '\n';
	}
}
