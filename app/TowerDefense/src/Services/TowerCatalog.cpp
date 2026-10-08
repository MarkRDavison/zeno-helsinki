#include <Services/TowerCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void TowerCatalog::load(const std::string& path, const WeaponCatalog& weapons)
	{
		loadFromText(hl::String::readFile(path), "towers.json", weapons);
	}

	void TowerCatalog::loadFromText(
		const std::string& text,
		const char* file,
		const WeaponCatalog& weapons)
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

			TowerDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.model = catalogJson::requireString(*row, "model", file);
			def.label = catalogJson::requireString(*row, "label", file);
			def.cost = catalogJson::requireIntAtLeast(*row, "cost", 1, file);
			def.range = catalogJson::requirePositive(*row, "range", file);

			def.weapons = catalogJson::parseWeaponSlots(*row, "weapons", file, weapons, true);

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

	const TowerDef* TowerCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<TowerDef>& TowerCatalog::all() const
	{
		return _defs;
	}
}
