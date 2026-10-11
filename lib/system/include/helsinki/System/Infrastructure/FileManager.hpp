#pragma once

#include <helsinki/System/Export.hpp>

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <string>

namespace hl
{

	class HELSINKI_SYSTEM_EXPORT FileManager : NonCopyable
	{
	public:
		void registerDirectory(const std::string& _directory);

		std::string resolvePath(const std::string& _relativePath) const;

	private:
		std::string m_Directory;
	};
}