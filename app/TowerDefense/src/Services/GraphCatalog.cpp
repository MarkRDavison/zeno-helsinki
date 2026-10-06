#include <Services/GraphCatalog.hpp>
#include <Services/CatalogJson.hpp>

namespace tower
{
	void collectGraphNodeRefs(const GraphPrereq& prereq, std::vector<std::string>& ids)
	{
		if (prereq.kind == GraphPrereqKind::Node)
		{
			ids.push_back(prereq.nodeId);
			return;
		}

		for (const auto& child : prereq.children)
		{
			collectGraphNodeRefs(child, ids);
		}
	}

	GraphPrereq parseGraphPrereqItem(const hl::JsonNode& item, const char* file);

	GraphPrereq parseGraphPrereqGroup(const hl::JsonNode& obj, const char* file)
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

		GraphPrereq prereq;
		prereq.kind = all != nullptr ? GraphPrereqKind::All : GraphPrereqKind::Any;
		for (const auto* child : list->children)
		{
			if (child == nullptr)
			{
				catalogJson::fail(std::string(file) + ": 'all'/'any' entries must not be null");
			}

			prereq.children.push_back(parseGraphPrereqItem(*child, file));
		}

		return prereq;
	}

	GraphPrereq parseGraphPrereqItem(const hl::JsonNode& item, const char* file)
	{
		if (item.type == hl::JsonNode::Type::ValueString)
		{
			if (item.content.empty())
			{
				catalogJson::fail(std::string(file) + ": prereq node id must be a non-empty string");
			}

			GraphPrereq prereq;
			prereq.kind = GraphPrereqKind::Node;
			prereq.nodeId = item.content;
			return prereq;
		}

		if (item.type == hl::JsonNode::Type::Object)
		{
			return parseGraphPrereqGroup(item, file);
		}

		catalogJson::fail(std::string(file) + ": prereq items must be node ids or all/any objects");
	}

	void validateGraphPrereqIds(
		const std::vector<GraphNode>& nodes,
		const std::unordered_map<std::string, std::size_t>& byId,
		const char* file)
	{
		std::vector<std::string> refs;
		for (const auto& node : nodes)
		{
			if (!node.prereq.has_value())
			{
				continue;
			}

			refs.clear();
			collectGraphNodeRefs(*node.prereq, refs);
			for (const auto& id : refs)
			{
				if (!byId.contains(id))
				{
					catalogJson::fail(std::string(file) + ": unknown prereq id '" + id + "'");
				}
			}
		}
	}
}
