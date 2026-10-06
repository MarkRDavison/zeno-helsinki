#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	struct LevelListEntry
	{
		std::string id;
		std::string label;
		std::string file;
	};

	class LevelsCatalog
	{
	public:
		void load(const std::string& path);
		const LevelListEntry* find(std::string_view id) const;
		const std::vector<LevelListEntry>& all() const;

	private:
		std::vector<LevelListEntry> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
