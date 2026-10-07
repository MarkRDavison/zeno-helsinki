#pragma once

#include <Services/StatusTypes.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class StatusCategoryCatalog
	{
	public:
		void load(const std::string& path);
		void loadFromText(const std::string& text, const char* file);
		const StatusCategoryDef* find(std::string_view id) const;
		const std::vector<StatusCategoryDef>& all() const;

	private:
		std::vector<StatusCategoryDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
