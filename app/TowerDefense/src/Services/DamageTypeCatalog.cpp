#include <Services/DamageTypeCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	namespace
	{
		const char* kFile = "damage-types.json";
	}

	void DamageTypeCatalog::load(const std::string& path)
	{
		loadFromText(hl::String::readFile(path), kFile);
	}

	void DamageTypeCatalog::loadFromText(const std::string& text, const char* file)
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

			DamageTypeDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.name = catalogJson::requireString(*row, "name", file);
			def.description = catalogJson::requireString(*row, "description", file);
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

		if (find("physical") == nullptr || find("fire") == nullptr || find("poison") == nullptr)
		{
			catalogJson::fail(std::string(file) + ": must define id 'physical', 'fire', and 'poison'");
		}
	}

	bool DamageTypeCatalog::contains(std::string_view id) const
	{
		return find(id) != nullptr;
	}

	const DamageTypeDef* DamageTypeCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<DamageTypeDef>& DamageTypeCatalog::all() const
	{
		return _defs;
	}
}
