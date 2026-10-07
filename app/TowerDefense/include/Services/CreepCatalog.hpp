#pragma once

#include <Services/WeaponCatalog.hpp>
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
		float range = 0.0f;
		std::unordered_map<std::string, float> resist;
		std::vector<WeaponSlot> slots;
	};

	class CreepCatalog
	{
	public:
		void load(const std::string& path, const DamageTypeCatalog& types);
		void load(
			const std::string& path,
			const DamageTypeCatalog& types,
			const WeaponCatalog& weapons);
		void loadFromText(const std::string& text, const char* file, const DamageTypeCatalog& types);
		void loadFromText(
			const std::string& text,
			const char* file,
			const DamageTypeCatalog& types,
			const WeaponCatalog& weapons);
		const CreepDef* find(std::string_view id) const;
		const std::vector<CreepDef>& all() const;

	private:
		std::vector<CreepDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
