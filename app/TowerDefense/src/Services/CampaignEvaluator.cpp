#include <Services/CampaignEvaluator.hpp>
#include <Services/GraphEvaluator.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tower
{
	namespace
	{
		bool prereqImpossible(
			const GraphPrereq& prereq,
			const std::unordered_set<std::string>& skipped)
		{
			switch (prereq.kind)
			{
			case GraphPrereqKind::Node:
				return skipped.contains(prereq.nodeId);
			case GraphPrereqKind::All:
				for (const auto& child : prereq.children)
				{
					if (prereqImpossible(child, skipped))
					{
						return true;
					}
				}
				return false;
			case GraphPrereqKind::Any:
				for (const auto& child : prereq.children)
				{
					if (!prereqImpossible(child, skipped))
					{
						return false;
					}
				}
				return true;
			}

			return false;
		}

		bool nodeImpossible(
			const CampaignNode& node,
			const std::unordered_set<std::string>& skipped)
		{
			if (!node.prereq.has_value())
			{
				return false;
			}

			return prereqImpossible(*node.prereq, skipped);
		}

		std::vector<GraphNode> graphNodes(const CampaignData& data)
		{
			std::vector<GraphNode> nodes;
			nodes.reserve(data.nodes.size());
			for (const auto& node : data.nodes)
			{
				nodes.push_back(GraphNode{ .id = node.id, .prereq = node.prereq });
			}

			return nodes;
		}
	}

	std::unordered_map<std::string, CampaignNodeState> evaluateCampaign(
		const CampaignData& data,
		const CampaignProgress& progress)
	{
		std::unordered_set<std::string> known;
		known.reserve(data.nodes.size());
		for (const auto& node : data.nodes)
		{
			known.insert(node.id);
		}

		std::unordered_set<std::string> cleared;
		for (const auto& id : progress.cleared)
		{
			if (known.contains(id))
			{
				cleared.insert(id);
			}
		}

		std::unordered_set<std::string> skipped;
		for (const auto& id : progress.skipped)
		{
			if (known.contains(id) && !cleared.contains(id))
			{
				skipped.insert(id);
			}
		}

		std::unordered_map<std::string, std::vector<std::size_t>> mutexGroups;
		for (std::size_t i = 0; i < data.nodes.size(); ++i)
		{
			const auto& node = data.nodes[i];
			if (node.mutex.has_value())
			{
				mutexGroups[*node.mutex].push_back(i);
			}
		}

		for (const auto& group : mutexGroups)
		{
			bool groupCleared = false;
			for (const auto index : group.second)
			{
				if (cleared.contains(data.nodes[index].id))
				{
					groupCleared = true;
					break;
				}
			}

			if (!groupCleared)
			{
				continue;
			}

			for (const auto index : group.second)
			{
				const auto& id = data.nodes[index].id;
				if (!cleared.contains(id))
				{
					skipped.insert(id);
				}
			}
		}

		bool changed = true;
		while (changed)
		{
			changed = false;
			for (const auto& node : data.nodes)
			{
				if (cleared.contains(node.id) || skipped.contains(node.id))
				{
					continue;
				}

				if (nodeImpossible(node, skipped))
				{
					skipped.insert(node.id);
					changed = true;
				}
			}
		}

		const auto graphStates = evaluateGraph(graphNodes(data), cleared, skipped);
		std::unordered_map<std::string, CampaignNodeState> states;
		states.reserve(data.nodes.size());
		for (const auto& node : data.nodes)
		{
			const auto it = graphStates.find(node.id);
			const auto graph = it != graphStates.end() ? it->second : GraphNodeState::Locked;
			switch (graph)
			{
			case GraphNodeState::Completed:
				states.emplace(node.id, CampaignNodeState::Cleared);
				break;
			case GraphNodeState::Skipped:
				states.emplace(node.id, CampaignNodeState::Skipped);
				break;
			case GraphNodeState::Available:
				states.emplace(node.id, CampaignNodeState::Available);
				break;
			case GraphNodeState::Locked:
			default:
				states.emplace(node.id, CampaignNodeState::Locked);
				break;
			}
		}

		return states;
	}
}
