#include <Services/ProjectileCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/StatusCatalog.hpp>

namespace tower
{
	namespace
	{
		const char* kFile = "projectiles.json";
	}

	void ProjectileCatalog::load(
		const std::string& path,
		const DamageTypeCatalog& types,
		const StatusCatalog& statuses)
	{
		loadFromText(hl::String::readFile(path), kFile, types, statuses);
	}

	void ProjectileCatalog::loadFromText(
		const std::string& text,
		const char* file,
		const DamageTypeCatalog& types,
		const StatusCatalog& statuses)
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

			ProjectileDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.model = catalogJson::requireString(*row, "model", file);
			def.damage = static_cast<float>(
				catalogJson::requireIntAtLeast(*row, "damage", 1, file));
			def.damageType = catalogJson::requireString(*row, "damageType", file);
			if (!types.contains(def.damageType))
			{
				catalogJson::fail(
					std::string(file) + ": unknown damageType '" + def.damageType + "'");
			}

			def.speed = catalogJson::requirePositive(*row, "speed", file);
			def.hitRadius = catalogJson::requirePositive(*row, "hitRadius", file);
			def.y = catalogJson::requirePositive(*row, "y", file);
			def.scale = catalogJson::requireVec3(*row, "scale", file, true);
			def.statuses = catalogJson::optionalStringArray(*row, "statuses", file);
			for (const auto& statusId : def.statuses)
			{
				if (statuses.find(statusId) == nullptr)
				{
					catalogJson::fail(std::string(file) + ": unknown status '" + statusId + "'");
				}
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

	const ProjectileDef* ProjectileCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<ProjectileDef>& ProjectileCatalog::all() const
	{
		return _defs;
	}
}
