#include <Services/ProjectileCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void ProjectileCatalog::load(const std::string& path)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireArrayRoot(doc, "projectiles.json");
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("projectiles.json: each entry must be an object");
			}

			ProjectileDef def;
			def.id = catalogJson::requireString(*row, "id", "projectiles.json");
			def.model = catalogJson::requireString(*row, "model", "projectiles.json");
			def.damage = catalogJson::requireIntAtLeast(*row, "damage", 1, "projectiles.json");
			def.speed = catalogJson::requirePositive(*row, "speed", "projectiles.json");
			def.hitRadius = catalogJson::requirePositive(*row, "hitRadius", "projectiles.json");
			def.y = catalogJson::requirePositive(*row, "y", "projectiles.json");
			def.scale = catalogJson::requireVec3(*row, "scale", "projectiles.json", true);
			if (_byId.contains(def.id))
			{
				catalogJson::fail("projectiles.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail("projectiles.json: catalog is empty");
		}
	}

	const ProjectileDef* ProjectileCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<ProjectileDef>& ProjectileCatalog::all() const
	{
		return _defs;
	}
}
