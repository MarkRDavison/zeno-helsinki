#include <Services/CampaignCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	namespace
	{
		const char* kFile = "campaign.json";

		void collectNodeRefs(const CampaignPrereq& prereq, std::vector<std::string>& ids)
		{
			if (prereq.kind == CampaignPrereqKind::Node)
			{
				ids.push_back(prereq.nodeId);
				return;
			}

			for (const auto& child : prereq.children)
			{
				collectNodeRefs(child, ids);
			}
		}

		CampaignPrereq parsePrereqGroup(const hl::JsonNode& obj, const char* file);

		CampaignPrereq parsePrereqItem(const hl::JsonNode& item, const char* file)
		{
			if (item.type == hl::JsonNode::Type::ValueString)
			{
				if (item.content.empty())
				{
					catalogJson::fail(std::string(file) + ": prereq node id must be a non-empty string");
				}

				CampaignPrereq prereq;
				prereq.kind = CampaignPrereqKind::Node;
				prereq.nodeId = item.content;
				return prereq;
			}

			if (item.type == hl::JsonNode::Type::Object)
			{
				return parsePrereqGroup(item, file);
			}

			catalogJson::fail(std::string(file) + ": prereq items must be node ids or all/any objects");
		}

		CampaignPrereq parsePrereqGroup(const hl::JsonNode& obj, const char* file)
		{
			const auto* all = catalogJson::findChild(obj, "all");
			const auto* any = catalogJson::findChild(obj, "any");
			if ((all == nullptr) == (any == nullptr))
			{
				catalogJson::fail(std::string(file) + ": prereq must be exactly one of 'all' or 'any'");
			}

			const auto* list = all != nullptr ? all : any;
			if (list->type != hl::JsonNode::Type::Array || list->children.empty())
			{
				catalogJson::fail(std::string(file) + ": 'all'/'any' must be a non-empty array");
			}

			CampaignPrereq prereq;
			prereq.kind = all != nullptr ? CampaignPrereqKind::All : CampaignPrereqKind::Any;
			for (const auto* child : list->children)
			{
				if (child == nullptr)
				{
					catalogJson::fail(std::string(file) + ": 'all'/'any' entries must not be null");
				}

				prereq.children.push_back(parsePrereqItem(*child, file));
			}

			return prereq;
		}
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

				def.prereq = parsePrereqGroup(*prereq, file);
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

		std::vector<std::string> refs;
		for (const auto& node : _data.nodes)
		{
			if (!node.prereq.has_value())
			{
				continue;
			}

			refs.clear();
			collectNodeRefs(*node.prereq, refs);
			for (const auto& id : refs)
			{
				if (!_byId.contains(id))
				{
					catalogJson::fail(std::string(file) + ": unknown prereq id '" + id + "'");
				}
			}
		}
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
