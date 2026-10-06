#include <Services/LevelsCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void LevelsCatalog::load(const std::string& path)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireArrayRoot(doc, "levels.json");
		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("levels.json: each entry must be an object");
			}

			LevelListEntry def;
			def.id = catalogJson::requireString(*row, "id", "levels.json");
			def.label = catalogJson::requireString(*row, "label", "levels.json");
			def.file = catalogJson::requireString(*row, "file", "levels.json");
			if (_byId.contains(def.id))
			{
				catalogJson::fail("levels.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			catalogJson::fail("levels.json: catalog is empty");
		}
	}

	const LevelListEntry* LevelsCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<LevelListEntry>& LevelsCatalog::all() const
	{
		return _defs;
	}
}
