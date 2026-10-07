#include <Services/StatusCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Combat.hpp>
#include <iostream>

namespace tower
{
	namespace
	{
		const char* kFile = "statuses.json";

		bool isKnownChannel(std::string_view channel, const DamageTypeCatalog& types)
		{
			if (channel == "speed" || channel == "health" || channel == "damage" || channel == "weakness")
			{
				return true;
			}

			constexpr std::string_view damageSuffix = "_damage";
			constexpr std::string_view resistSuffix = "_resistance";
			const std::string value{ channel };
			if (value.size() > damageSuffix.size()
				&& value.ends_with(damageSuffix))
			{
				return types.contains(value.substr(0, value.size() - damageSuffix.size()));
			}

			if (value.size() > resistSuffix.size()
				&& value.ends_with(resistSuffix))
			{
				return types.contains(value.substr(0, value.size() - resistSuffix.size()));
			}

			return false;
		}
	}

	void StatusCatalog::load(
		const std::string& path,
		const StatusCategoryCatalog& categories,
		const DamageTypeCatalog& types)
	{
		loadFromText(hl::String::readFile(path), kFile, categories, types);
	}

	void StatusCatalog::loadFromText(
		const std::string& text,
		const char* file,
		const StatusCategoryCatalog& categories,
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

			StatusDef def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.category = catalogJson::requireString(*row, "category", file);
			const auto* category = categories.find(def.category);
			if (category == nullptr)
			{
				catalogJson::fail(std::string(file) + ": unknown category '" + def.category + "'");
			}

			def.kind = category->kind;
			def.duration = catalogJson::requirePositive(*row, "duration", file);
			def.score = catalogJson::optionalInt(*row, "score", 0, file);
			def.cap = catalogJson::optionalIntAtLeast(*row, "cap", 1, 1, file);
			if (def.kind == StatusKind::Dot)
			{
				def.interval = catalogJson::requirePositive(*row, "interval", file);
				def.tickDamage = static_cast<float>(
					catalogJson::requireIntAtLeast(*row, "tickDamage", 0, file));
				def.damageType = catalogJson::requireString(*row, "damageType", file);
				if (!types.contains(def.damageType))
				{
					catalogJson::fail(
						std::string(file) + ": unknown damageType '" + def.damageType + "'");
				}
			}
			else
			{
				def.channel = catalogJson::requireString(*row, "channel", file);
				if (!isKnownChannel(def.channel, types))
				{
					catalogJson::fail(std::string(file) + ": unknown channel '" + def.channel + "'");
				}

				const float raw = catalogJson::requireAnyNumber(
					catalogJson::field(*row, "magnitude"),
					file,
					"magnitude");
				if (isUnitChannel(def.channel))
				{
					const auto clamped = clampUnit(raw);
					if (clamped.warned)
					{
						std::clog << file << ": status '" << def.id
							<< "' magnitude clamped to 0..1\n";
					}

					def.magnitude = clamped.value;
				}
				else
				{
					def.magnitude = raw;
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

		if (find("slow") == nullptr
			|| find("poison") == nullptr
			|| find("burn") == nullptr
			|| find("weakness") == nullptr)
		{
			catalogJson::fail(std::string(file) + ": must define id 'slow', 'poison', 'burn', and 'weakness'");
		}
	}

	const StatusDef* StatusCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<StatusDef>& StatusCatalog::all() const
	{
		return _defs;
	}
}
