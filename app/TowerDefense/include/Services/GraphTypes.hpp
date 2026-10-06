#pragma once

#include <optional>
#include <string>
#include <vector>

namespace tower
{
	enum class GraphPrereqKind
	{
		All,
		Any,
		Node
	};

	struct GraphPrereq
	{
		GraphPrereqKind kind = GraphPrereqKind::All;
		std::string nodeId;
		std::vector<GraphPrereq> children;
	};

	struct GraphNode
	{
		std::string id;
		std::optional<GraphPrereq> prereq;
	};

	enum class GraphNodeState
	{
		Locked,
		Available,
		Completed,
		Skipped
	};
}
