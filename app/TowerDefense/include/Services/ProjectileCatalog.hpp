#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class DamageTypeCatalog;

	struct ProjectileDef
	{
		std::string id;
		std::string model;
		float damage = 1.0f;
		std::string damageType;
		float speed = 0.0f;
		float hitRadius = 0.0f;
		float y = 0.0f;
		glm::vec3 scale{ 1.0f };
	};

	class ProjectileCatalog
	{
	public:
		void load(const std::string& path, const DamageTypeCatalog& types);
		void loadFromText(const std::string& text, const char* file, const DamageTypeCatalog& types);
		const ProjectileDef* find(std::string_view id) const;
		const std::vector<ProjectileDef>& all() const;

	private:
		std::vector<ProjectileDef> _defs;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
