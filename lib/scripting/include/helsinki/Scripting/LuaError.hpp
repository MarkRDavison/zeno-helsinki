#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace hl::scripting
{
	class LuaError : public std::runtime_error
	{
	public:
		explicit LuaError(std::string message, std::string traceback = {})
			: std::runtime_error(combine(message, traceback))
			, _message(std::move(message))
			, _traceback(std::move(traceback))
		{
		}

		const std::string& message() const
		{
			return _message;
		}

		const std::string& traceback() const
		{
			return _traceback;
		}

	private:
		static std::string combine(const std::string& message, const std::string& traceback)
		{
			if (traceback.empty() || traceback == message)
			{
				return message;
			}
			return message + "\n" + traceback;
		}

		std::string _message;
		std::string _traceback;
	};
}
