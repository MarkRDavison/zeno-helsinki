#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	struct DamageTypeDef
	{
		std::string id;
		std::string name;
		std::string description;
	};

	class DamageTypeCatalog
	{
	public:
		void load(const std::string& path);
		void loadFromText(const std::string& text, const char* file);
		bool contains(std::string_view id) const;
		const DamageTypeDef* find(std::string_view id) const;
		const std::vector<DamageTypeDef>& all() const;

	private:
		std::vector<DamageTypeDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
