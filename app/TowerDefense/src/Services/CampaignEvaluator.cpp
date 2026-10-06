#include <Services/CampaignEvaluator.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tower
{
	namespace
	{
		bool prereqSatisfied(
			const CampaignPrereq& prereq,
			const std::unordered_set<std::string>& cleared)
		{
			switch (prereq.kind)
			{
			case CampaignPrereqKind::Node:
				return cleared.contains(prereq.nodeId);
			case CampaignPrereqKind::All:
				for (const auto& child : prereq.children)
				{
					if (!prereqSatisfied(child, cleared))
					{
						return false;
					}
				}
				return true;
			case CampaignPrereqKind::Any:
				for (const auto& child : prereq.children)
				{
					if (prereqSatisfied(child, cleared))
					{
						return true;
					}
				}
				return false;
			}

			return false;
		}

		bool nodeSatisfied(
			const CampaignNode& node,
			const std::unordered_set<std::string>& cleared)
		{
			if (!node.prereq.has_value())
			{
				return true;
			}

			return prereqSatisfied(*node.prereq, cleared);
		}

		bool prereqImpossible(
			const CampaignPrereq& prereq,
			const std::unordered_set<std::string>& skipped)
		{
			switch (prereq.kind)
			{
			case CampaignPrereqKind::Node:
				return skipped.contains(prereq.nodeId);
			case CampaignPrereqKind::All:
				for (const auto& child : prereq.children)
				{
					if (prereqImpossible(child, skipped))
					{
						return true;
					}
				}
				return false;
			case CampaignPrereqKind::Any:
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

		std::unordered_map<std::string, CampaignNodeState> states;
		states.reserve(data.nodes.size());
		for (const auto& node : data.nodes)
		{
			if (cleared.contains(node.id))
			{
				states.emplace(node.id, CampaignNodeState::Cleared);
			}
			else if (skipped.contains(node.id))
			{
				states.emplace(node.id, CampaignNodeState::Skipped);
			}
			else if (nodeSatisfied(node, cleared))
			{
				states.emplace(node.id, CampaignNodeState::Available);
			}
			else
			{
				states.emplace(node.id, CampaignNodeState::Locked);
			}
		}

		return states;
	}
}
