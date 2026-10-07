#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void WeaponCatalog::load(const std::string& path, const ProjectileCatalog& projectiles)
	{
		loadFromText(hl::String::readFile(path), "weapons.json", projectiles);
	}

	void WeaponCatalog::loadFromText(
		const std::string& text,
		const char* file,
		const ProjectileCatalog& projectiles)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(text);
		catalogJson::requireArrayRoot(doc, file);
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail(std::string(file) + ": each entry must be an object");
			}

			WeaponDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.projectile = catalogJson::requireString(*row, "projectile", file);
			def.fireCooldown = catalogJson::requirePositive(*row, "fireCooldown", file);
			if (projectiles.find(def.projectile) == nullptr)
			{
				catalogJson::fail(std::string(file) + ": unknown projectile '" + def.projectile + "'");
			}

			if (_byId.contains(def.id))
			{
				catalogJson::fail(std::string(file) + ": duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail(std::string(file) + ": catalog is empty");
		}
	}

	const WeaponDef* WeaponCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<WeaponDef>& WeaponCatalog::all() const
	{
		return _defs;
	}
}
