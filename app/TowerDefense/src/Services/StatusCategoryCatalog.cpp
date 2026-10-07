#include <Services/StatusCategoryCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	namespace
	{
		const char* kFile = "status-categories.json";

		StatusKind parseKind(const std::string& kind, const char* file)
		{
			if (kind == "dot")
			{
				return StatusKind::Dot;
			}

			if (kind == "stat")
			{
				return StatusKind::Stat;
			}

			catalogJson::fail(std::string(file) + ": unknown kind '" + kind + "'");
		}
	}

	void StatusCategoryCatalog::load(const std::string& path)
	{
		loadFromText(hl::String::readFile(path), kFile);
	}

	void StatusCategoryCatalog::loadFromText(const std::string& text, const char* file)
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

			StatusCategoryDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.kind = parseKind(catalogJson::requireString(*row, "kind", file), file);
			def.color = catalogJson::requireColor01(*row, "color", file);
			def.cap = catalogJson::optionalIntAtLeast(*row, "cap", 1, 1, file);
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

		if (find("slow") == nullptr
			|| find("poison") == nullptr
			|| find("burn") == nullptr
			|| find("weakness") == nullptr)
		{
			catalogJson::fail(std::string(file) + ": must define id 'slow', 'poison', 'burn', and 'weakness'");
		}
	}

	const StatusCategoryDef* StatusCategoryCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<StatusCategoryDef>& StatusCategoryCatalog::all() const
	{
		return _defs;
	}
}
