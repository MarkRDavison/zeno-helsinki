#include <Services/EntityCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void EntityCatalog::load(const std::string& path)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireArrayRoot(doc, "entities.json");
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("entities.json: each entry must be an object");
			}

			EntityDef def;
			def.id = catalogJson::requireString(*row, "id", "entities.json");
			def.model = catalogJson::requireString(*row, "model", "entities.json");
			const auto size = catalogJson::requireSize(*row, "entities.json");
			def.sizeX = size.first;
			def.sizeZ = size.second;
			if (_byId.contains(def.id))
			{
				catalogJson::fail("entities.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail("entities.json: catalog is empty");
		}
	}

	const EntityDef* EntityCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<EntityDef>& EntityCatalog::all() const
	{
		return _defs;
	}
}
