#include <helsinki/System/RuntimeRoot.hpp>

#include <filesystem>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <vector>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

namespace hl
{

	std::string executableDirectory()
	{
#ifdef _WIN32
		wchar_t buffer[MAX_PATH]{};
		const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
		if (length == 0 || length >= MAX_PATH)
		{
			return {};
		}
		return std::filesystem::path(buffer).parent_path().generic_string();
#elif defined(__APPLE__)
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		std::vector<char> buffer(size);
		if (_NSGetExecutablePath(buffer.data(), &size) != 0)
		{
			return {};
		}
		return std::filesystem::weakly_canonical(std::filesystem::path(buffer.data())).parent_path().generic_string();
#else
		char buffer[PATH_MAX]{};
		const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (length <= 0)
		{
			return {};
		}
		buffer[length] = '\0';
		return std::filesystem::path(buffer).parent_path().generic_string();
#endif
	}

	std::string runtimeRootPath(const std::string& bakedSourceDirectory)
	{
		const auto exeDir = std::filesystem::path(executableDirectory());
		if (std::filesystem::exists(exeDir / "data" / "config.json"))
		{
			return exeDir.generic_string();
		}

		return bakedSourceDirectory;
	}

}
