#pragma once

#include <Services/WeaponCatalog.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	struct TowerDef
	{
		std::string id;
		std::string model;
		std::string label;
		int cost = 0;
		float range = 0.0f;
		std::vector<WeaponSlot> weapons;
	};

	class TowerCatalog
	{
	public:
		void load(const std::string& path, const WeaponCatalog& weapons);
		const TowerDef* find(std::string_view id) const;
		const std::vector<TowerDef>& all() const;

	private:
		std::vector<TowerDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
