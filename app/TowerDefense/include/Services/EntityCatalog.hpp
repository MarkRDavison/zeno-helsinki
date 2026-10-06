#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	struct EntityDef
	{
		std::string id;
		std::string model;
		int sizeX = 1;
		int sizeZ = 1;
	};

	class EntityCatalog
	{
	public:
		void load(const std::string& path);
		const EntityDef* find(std::string_view id) const;
		const std::vector<EntityDef>& all() const;

	private:
		std::vector<EntityDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
