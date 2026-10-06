#pragma once

#include <Services/GraphTypes.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tower
{
	std::unordered_map<std::string, GraphNodeState> evaluateGraph(
		const std::vector<GraphNode>& nodes,
		const std::unordered_set<std::string>& completed,
		const std::unordered_set<std::string>& skipped = {});

	std::unordered_map<std::string, int> graphNodeDepths(const std::vector<GraphNode>& nodes);
}
