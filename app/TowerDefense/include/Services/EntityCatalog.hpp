#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class DamageTypeCatalog;

	struct EntityDef
	{
		std::string id;
		std::string model;
		int sizeX = 1;
		int sizeZ = 1;
		float health = 0.0f;
		std::unordered_map<std::string, float> resist;
	};

	class EntityCatalog
	{
	public:
		void load(const std::string& path, const DamageTypeCatalog& types);
		void loadFromText(const std::string& text, const char* file, const DamageTypeCatalog& types);
		const EntityDef* find(std::string_view id) const;
		const std::vector<EntityDef>& all() const;

	private:
		std::vector<EntityDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
