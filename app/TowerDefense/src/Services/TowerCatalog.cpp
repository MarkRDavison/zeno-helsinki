#include <Services/TowerCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void TowerCatalog::load(const std::string& path, const WeaponCatalog& weapons)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireArrayRoot(doc, "towers.json");
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("towers.json: each entry must be an object");
			}

			TowerDef def;
			def.id = catalogJson::requireString(*row, "id", "towers.json");
			def.model = catalogJson::requireString(*row, "model", "towers.json");
			def.label = catalogJson::requireString(*row, "label", "towers.json");
			def.cost = catalogJson::requireIntAtLeast(*row, "cost", 1, "towers.json");
			def.range = catalogJson::requirePositive(*row, "range", "towers.json");

			const auto& weaponsNode = catalogJson::field(*row, "weapons");
			if (weaponsNode.type != hl::JsonNode::Type::Array || weaponsNode.children.empty())
			{
				catalogJson::fail("towers.json: '" + def.id + "' needs a non-empty weapons array");
			}

			for (const auto* slotRow : weaponsNode.children)
			{
				if (slotRow == nullptr || slotRow->type != hl::JsonNode::Type::Object)
				{
					catalogJson::fail("towers.json: each weapon slot must be an object");
				}

				TowerWeaponSlot slot;
				slot.id = catalogJson::requireString(*slotRow, "id", "towers.json");
				slot.offset = catalogJson::requireVec3(*slotRow, "offset", "towers.json", false);
				if (weapons.find(slot.id) == nullptr)
				{
					catalogJson::fail("towers.json: unknown weapon id '" + slot.id + "'");
				}

				def.weapons.push_back(std::move(slot));
			}

			if (_byId.contains(def.id))
			{
				catalogJson::fail("towers.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail("towers.json: catalog is empty");
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
