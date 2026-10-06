#include <Services/ResearchCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/GraphCatalog.hpp>

namespace tower
{
	namespace
	{
		const char* kFile = "research.json";

		ResearchEffect parseEffect(const hl::JsonNode& row, const char* file)
		{
			const auto* effect = catalogJson::findChild(row, "effect");
			if (effect == nullptr)
			{
				return {};
			}

			if (effect->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail(std::string(file) + ": 'effect' must be an object");
			}

			ResearchEffect parsed;
			if (const auto* tower = catalogJson::findChild(*effect, "tower"); tower != nullptr)
			{
				if (tower->type != hl::JsonNode::Type::ValueString || tower->content.empty())
				{
					catalogJson::fail(std::string(file) + ": 'tower' must be a non-empty string");
				}

				parsed.tower = tower->content;
			}

			if (const auto* gold = catalogJson::findChild(*effect, "startingGoldRank"); gold != nullptr)
			{
				if (gold->type != hl::JsonNode::Type::ValueInteger || gold->integer < 1)
				{
					catalogJson::fail(std::string(file) + ": 'startingGoldRank' must be an integer >= 1");
				}

				parsed.startingGoldRank = gold->integer;
			}

			if (const auto* fire = catalogJson::findChild(*effect, "fireRateRank"); fire != nullptr)
			{
				if (fire->type != hl::JsonNode::Type::ValueInteger || fire->integer < 1)
				{
					catalogJson::fail(std::string(file) + ": 'fireRateRank' must be an integer >= 1");
				}

				parsed.fireRateRank = fire->integer;
			}

			return parsed;
		}
	}

	void ResearchCatalog::load(const std::string& path)
	{
		loadFromText(hl::String::readFile(path), kFile);
	}

	void ResearchCatalog::loadFromText(const std::string& text, const char* file)
	{
		_data = {};
		_byId.clear();

		const auto doc = hl::Json::parseFromText(text);
		catalogJson::requireObjectRoot(doc, file);

		_data.version = catalogJson::requireIntAtLeast(*doc.m_Root, "version", 1, file);
		const auto& nodes = catalogJson::field(*doc.m_Root, "nodes");
		if (nodes.type != hl::JsonNode::Type::Array || nodes.children.empty())
		{
			catalogJson::fail(std::string(file) + ": 'nodes' must be a non-empty array");
		}

		for (const auto* row : nodes.children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail(std::string(file) + ": each node must be an object");
			}

			ResearchNode def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.label = catalogJson::requireString(*row, "label", file);
			if (const auto* cost = catalogJson::findChild(*row, "cost"); cost != nullptr)
			{
				if (cost->type != hl::JsonNode::Type::ValueInteger || cost->integer < 0)
				{
					catalogJson::fail(std::string(file) + ": 'cost' must be an integer >= 0");
				}

				def.cost = cost->integer;
			}

			if (const auto* start = catalogJson::findChild(*row, "start"); start != nullptr)
			{
				if (start->type != hl::JsonNode::Type::ValueBoolean)
				{
					catalogJson::fail(std::string(file) + ": 'start' must be a boolean");
				}

				def.start = start->boolean;
			}

			if (const auto* prereq = catalogJson::findChild(*row, "prereq"); prereq != nullptr)
			{
				if (prereq->type != hl::JsonNode::Type::Object)
				{
					catalogJson::fail(std::string(file) + ": 'prereq' must be an object");
				}

				def.prereq = parseGraphPrereqGroup(*prereq, file);
			}

			def.effect = parseEffect(*row, file);

			if (_byId.contains(def.id))
			{
				catalogJson::fail(std::string(file) + ": duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _data.nodes.size());
			_data.nodes.push_back(std::move(def));
		}

		validateGraphPrereqIds(researchGraphNodes(_data), _byId, file);
	}

	const ResearchData& ResearchCatalog::data() const
	{
		return _data;
	}

	const ResearchNode* ResearchCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_data.nodes[it->second];
	}

	const std::vector<ResearchNode>& ResearchCatalog::nodes() const
	{
		return _data.nodes;
	}
}
