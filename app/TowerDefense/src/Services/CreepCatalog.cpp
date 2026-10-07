#include <Services/CreepCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Combat.hpp>
#include <iostream>

namespace tower
{
	namespace
	{
		const char* kFile = "creeps.json";

		std::unordered_map<std::string, float> parseResist(
			const hl::JsonNode& row,
			const std::string& creepId,
			const char* file,
			const DamageTypeCatalog& types)
		{
			std::unordered_map<std::string, float> resist;
			const auto* node = catalogJson::optionalObject(row, "resist", file);
			if (node == nullptr)
			{
				return resist;
			}

			for (const auto* child : node->children)
			{
				if (child == nullptr)
				{
					continue;
				}

				if (child->name.empty())
				{
					catalogJson::fail(std::string(file) + ": resist keys must be type ids");
				}

				if (!types.contains(child->name))
				{
					catalogJson::fail(
						std::string(file) + ": unknown resist type '" + child->name + "'");
				}

				const float raw = catalogJson::requireAnyNumber(*child, file, child->name.c_str());
				const auto clamped = clampResist(raw);
				if (clamped.warned)
				{
					std::clog << file << ": creep '" << creepId << "' resist '" << child->name
						<< "' " << raw << " clamped to 1.0\n";
				}

				resist[child->name] = clamped.value;
			}

			return resist;
		}
	}

	void CreepCatalog::load(const std::string& path, const DamageTypeCatalog& types)
	{
		loadFromText(hl::String::readFile(path), kFile, types);
	}

	void CreepCatalog::loadFromText(
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

			CreepDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.model = catalogJson::requireString(*row, "model", file);
			def.health = static_cast<float>(
				catalogJson::requireIntAtLeast(*row, "health", 1, file));
			def.speed = catalogJson::requirePositive(*row, "speed", file);
			def.scale = catalogJson::optionalPositive(*row, "scale", 0.4f, file);
			def.resist = parseResist(*row, def.id, file, types);
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

		if (find("runner") == nullptr || find("tank") == nullptr)
		{
			catalogJson::fail(std::string(file) + ": must define id 'runner' and 'tank'");
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
