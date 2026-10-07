#pragma once

#include <Services/StatusTypes.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class DamageTypeCatalog;
	class StatusCategoryCatalog;

	class StatusCatalog
	{
	public:
		void load(
			const std::string& path,
			const StatusCategoryCatalog& categories,
			const DamageTypeCatalog& types);
		void loadFromText(
			const std::string& text,
			const char* file,
			const StatusCategoryCatalog& categories,
			const DamageTypeCatalog& types);
		const StatusDef* find(std::string_view id) const;
		const std::vector<StatusDef>& all() const;

	private:
		std::vector<StatusDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
