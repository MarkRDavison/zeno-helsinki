#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class DamageTypeCatalog;

	struct CreepDef
	{
		std::string id;
		std::string model;
		float health = 0.0f;
		float speed = 0.0f;
		float scale = 0.4f;
		std::unordered_map<std::string, float> resist;
	};

	class CreepCatalog
	{
	public:
		void load(const std::string& path, const DamageTypeCatalog& types);
		void loadFromText(const std::string& text, const char* file, const DamageTypeCatalog& types);
		const CreepDef* find(std::string_view id) const;
		const std::vector<CreepDef>& all() const;

	private:
		std::vector<CreepDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
