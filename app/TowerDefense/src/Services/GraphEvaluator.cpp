#include <Services/GraphEvaluator.hpp>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace tower
{
	namespace
	{
		bool prereqSatisfied(
			const GraphPrereq& prereq,
			const std::unordered_set<std::string>& completed)
		{
			switch (prereq.kind)
			{
			case GraphPrereqKind::Node:
				return completed.contains(prereq.nodeId);
			case GraphPrereqKind::All:
				for (const auto& child : prereq.children)
				{
					if (!prereqSatisfied(child, completed))
					{
						return false;
					}
				}
				return true;
			case GraphPrereqKind::Any:
				for (const auto& child : prereq.children)
				{
					if (prereqSatisfied(child, completed))
					{
						return true;
					}
				}
				return false;
			}

			return false;
		}

		bool nodeSatisfied(
			const GraphNode& node,
			const std::unordered_set<std::string>& completed)
		{
			if (!node.prereq.has_value())
			{
				return true;
			}

			return prereqSatisfied(*node.prereq, completed);
		}

		int nodeDepth(
			const GraphNode& node,
			const std::unordered_map<std::string, const GraphNode*>& byId,
			std::unordered_map<std::string, int>& memo,
			std::unordered_set<std::string>& visiting)
		{
			if (const auto it = memo.find(node.id); it != memo.end())
			{
				return it->second;
			}

			if (!visiting.insert(node.id).second)
			{
				return 0;
			}

			int depth = 0;
			if (node.prereq.has_value())
			{
				std::vector<std::string> refs;
				std::function<void(const GraphPrereq&)> walk = [&](const GraphPrereq& prereq)
				{
					if (prereq.kind == GraphPrereqKind::Node)
					{
						refs.push_back(prereq.nodeId);
						return;
					}

					for (const auto& child : prereq.children)
					{
						walk(child);
					}
				};
				walk(*node.prereq);
				for (const auto& id : refs)
				{
					const auto found = byId.find(id);
					if (found == byId.end())
					{
						continue;
					}

					depth = std::max(depth, nodeDepth(*found->second, byId, memo, visiting) + 1);
				}
			}

			visiting.erase(node.id);
			memo.emplace(node.id, depth);
			return depth;
		}
	}

	std::unordered_map<std::string, GraphNodeState> evaluateGraph(
		const std::vector<GraphNode>& nodes,
		const std::unordered_set<std::string>& completed,
		const std::unordered_set<std::string>& skipped)
	{
		std::unordered_set<std::string> known;
		known.reserve(nodes.size());
		for (const auto& node : nodes)
		{
			known.insert(node.id);
		}

		std::unordered_set<std::string> done;
		for (const auto& id : completed)
		{
			if (known.contains(id))
			{
				done.insert(id);
			}
		}

		std::unordered_set<std::string> skip;
		for (const auto& id : skipped)
		{
			if (known.contains(id) && !done.contains(id))
			{
				skip.insert(id);
			}
		}

		std::unordered_map<std::string, GraphNodeState> states;
		states.reserve(nodes.size());
		for (const auto& node : nodes)
		{
			if (done.contains(node.id))
			{
				states.emplace(node.id, GraphNodeState::Completed);
			}
			else if (skip.contains(node.id))
			{
				states.emplace(node.id, GraphNodeState::Skipped);
			}
			else if (nodeSatisfied(node, done))
			{
				states.emplace(node.id, GraphNodeState::Available);
			}
			else
			{
				states.emplace(node.id, GraphNodeState::Locked);
			}
		}

		return states;
	}

	std::unordered_map<std::string, int> graphNodeDepths(const std::vector<GraphNode>& nodes)
	{
		std::unordered_map<std::string, const GraphNode*> byId;
		byId.reserve(nodes.size());
		for (const auto& node : nodes)
		{
			byId.emplace(node.id, &node);
		}

		std::unordered_map<std::string, int> depths;
		std::unordered_set<std::string> visiting;
		for (const auto& node : nodes)
		{
			nodeDepth(node, byId, depths, visiting);
		}

		return depths;
	}
}
