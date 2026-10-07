#pragma once

#include <Combat.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

namespace tower
{
	inline std::unordered_map<std::string, float> parseResist(
		const hl::JsonNode& row,
		const std::string& ownerId,
		const char* file,
		const char* ownerKind,
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
				catalogJson::fail(std::string(file) + ": unknown resist type '" + child->name + "'");
			}

			const float raw = catalogJson::requireAnyNumber(*child, file, child->name.c_str());
			const auto clamped = clampResist(raw);
			if (clamped.warned)
			{
				std::clog << file << ": " << ownerKind << " '" << ownerId << "' resist '" << child->name
					<< "' " << raw << " clamped to 1.0\n";
			}

			resist[child->name] = clamped.value;
		}

		return resist;
	}
}
