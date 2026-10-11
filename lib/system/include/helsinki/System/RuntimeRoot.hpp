#pragma once

#include <helsinki/System/Export.hpp>

#include <string>

namespace hl
{

	HELSINKI_SYSTEM_EXPORT std::string executableDirectory();

	// Exe directory if `<exe>/data/config.json` exists, otherwise the CMake-baked source directory.
	HELSINKI_SYSTEM_EXPORT std::string runtimeRootPath(const std::string& bakedSourceDirectory);

}
