#include <Services/CampaignCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/GraphCatalog.hpp>

namespace tower
{
	namespace
	{
		const char* kFile = "campaign.json";
	}

	void CampaignCatalog::load(const std::string& path)
	{
		loadFromText(hl::String::readFile(path), kFile);
	}

	void CampaignCatalog::loadFromText(const std::string& text, const char* file)
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

			CampaignNode def;
			def.id = catalogJson::requireString(*row, "id", file);
			def.level = catalogJson::requireString(*row, "level", file);
			if (const auto* prereq = catalogJson::findChild(*row, "prereq"); prereq != nullptr)
			{
				if (prereq->type != hl::JsonNode::Type::Object)
				{
					catalogJson::fail(std::string(file) + ": 'prereq' must be an object");
				}

				def.prereq = parseGraphPrereqGroup(*prereq, file);
			}

			if (const auto* mutex = catalogJson::findChild(*row, "mutex"); mutex != nullptr)
			{
				if (mutex->type != hl::JsonNode::Type::ValueString || mutex->content.empty())
				{
					catalogJson::fail(std::string(file) + ": 'mutex' must be a non-empty string");
				}

				def.mutex = mutex->content;
			}

			if (_byId.contains(def.id))
			{
				catalogJson::fail(std::string(file) + ": duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _data.nodes.size());
			_data.nodes.push_back(std::move(def));
		}

		std::vector<GraphNode> graph;
		graph.reserve(_data.nodes.size());
		for (const auto& node : _data.nodes)
		{
			graph.push_back(GraphNode{ .id = node.id, .prereq = node.prereq });
		}

		validateGraphPrereqIds(graph, _byId, file);
	}

	const CampaignData& CampaignCatalog::data() const
	{
		return _data;
	}

	const CampaignNode* CampaignCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_data.nodes[it->second];
	}

	const std::vector<CampaignNode>& CampaignCatalog::nodes() const
	{
		return _data.nodes;
	}
}
