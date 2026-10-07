#include <Services/EntityCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <CatalogResist.hpp>

namespace tower
{
	void EntityCatalog::load(const std::string& path, const DamageTypeCatalog& types)
	{
		loadFromText(hl::String::readFile(path), "entities.json", types);
	}

	void EntityCatalog::loadFromText(
		const std::string& text,
		const char* file,
		const DamageTypeCatalog& types)
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

			EntityDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.model = catalogJson::requireString(*row, "model", file);
			const auto size = catalogJson::requireSize(*row, file);
			def.sizeX = size.first;
			def.sizeZ = size.second;
			def.health = catalogJson::optionalNonNegative(*row, "health", 0.0f, file);
			def.resist = parseResist(*row, def.id, file, "entity", types);
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
