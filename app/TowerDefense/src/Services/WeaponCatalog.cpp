#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void WeaponCatalog::load(const std::string& path, const ProjectileCatalog& projectiles)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireArrayRoot(doc, "weapons.json");
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("weapons.json: each entry must be an object");
			}

			WeaponDef def;
			def.id = catalogJson::requireString(*row, "id", "weapons.json");
			def.projectile = catalogJson::requireString(*row, "projectile", "weapons.json");
			def.fireCooldown = catalogJson::requirePositive(*row, "fireCooldown", "weapons.json");
			if (projectiles.find(def.projectile) == nullptr)
			{
				catalogJson::fail("weapons.json: unknown projectile '" + def.projectile + "'");
			}

			if (_byId.contains(def.id))
			{
				catalogJson::fail("weapons.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail("weapons.json: catalog is empty");
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
