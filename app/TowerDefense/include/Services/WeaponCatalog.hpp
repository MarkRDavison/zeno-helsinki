#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class ProjectileCatalog;

	struct WeaponSlot
	{
		std::string id;
		glm::vec3 offset{ 0.0f };
	};

	struct WeaponDef
	{
		std::string id;
		std::string projectile;
		float fireCooldown = 0.0f;
	};

	class WeaponCatalog
	{
	public:
		void load(const std::string& path, const ProjectileCatalog& projectiles);
		void loadFromText(
			const std::string& text,
			const char* file,
			const ProjectileCatalog& projectiles);
		const WeaponDef* find(std::string_view id) const;
		const std::vector<WeaponDef>& all() const;

	private:
		std::vector<WeaponDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
