#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	struct CreepDef
	{
		std::string id;
		std::string model;
		int health = 0;
		float speed = 0.0f;
		float scale = 0.4f;
	};

	class CreepCatalog
	{
	public:
		void load(const std::string& path);
		const CreepDef* find(std::string_view id) const;
		const std::vector<CreepDef>& all() const;

	private:
		std::vector<CreepDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
