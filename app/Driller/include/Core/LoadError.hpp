#pragma once

#include <stdexcept>
#include <string>

namespace drl
{

	class LoadError : public std::runtime_error
	{
	public:
		explicit LoadError(const std::string& message)
			: std::runtime_error(message)
		{
		}
	};

}
