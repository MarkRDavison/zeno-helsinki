#include <Services/CreepCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void CreepCatalog::load(const std::string& path)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireArrayRoot(doc, "creeps.json");
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("creeps.json: each entry must be an object");
			}

			CreepDef def;
			def.id = catalogJson::requireString(*row, "id", "creeps.json");
			def.model = catalogJson::requireString(*row, "model", "creeps.json");
			def.health = catalogJson::requireIntAtLeast(*row, "health", 1, "creeps.json");
			def.speed = catalogJson::requirePositive(*row, "speed", "creeps.json");
			def.scale = catalogJson::optionalPositive(*row, "scale", 0.4f, "creeps.json");
			if (_byId.contains(def.id))
			{
				catalogJson::fail("creeps.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail("creeps.json: catalog is empty");
		}

		if (find("runner") == nullptr || find("tank") == nullptr)
		{
			catalogJson::fail("creeps.json: must define id 'runner' and 'tank'");
		}
	}

	const CreepDef* CreepCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<CreepDef>& CreepCatalog::all() const
	{
		return _defs;
	}
}
