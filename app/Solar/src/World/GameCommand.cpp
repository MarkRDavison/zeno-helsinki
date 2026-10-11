#include <World/GameCommand.hpp>
#include <algorithm>
#include <map>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace slr
{
namespace
{

	std::vector<long long> shortestLanePath(const WorldData& world, long long fromId, long long toId)
	{
		if (world.systemById(fromId) == nullptr || world.systemById(toId) == nullptr)
		{
			return {};
		}

		if (fromId == toId)
		{
			return { fromId };
		}

		std::map<long long, std::vector<long long>> adj;
		for (const auto& lane : world.hyperlanes)
		{
			adj[lane.a].push_back(lane.b);
			adj[lane.b].push_back(lane.a);
		}

		for (auto& entry : adj)
		{
			std::sort(entry.second.begin(), entry.second.end());
		}

		std::queue<long long> frontier;
		std::unordered_map<long long, long long> parent;
		frontier.push(fromId);
		parent.emplace(fromId, fromId);

		while (!frontier.empty())
		{
			const long long current = frontier.front();
			frontier.pop();
			if (current == toId)
			{
				break;
			}

			const auto found = adj.find(current);
			if (found == adj.end())
			{
				continue;
			}

			for (const long long next : found->second)
			{
				if (parent.contains(next))
				{
					continue;
				}

				parent.emplace(next, current);
				frontier.push(next);
			}
		}

		if (!parent.contains(toId))
		{
			return {};
		}

		std::vector<long long> path;
		for (long long at = toId; at != fromId; at = parent[at])
		{
			path.push_back(at);
		}
		path.push_back(fromId);
		std::reverse(path.begin(), path.end());
		return path;
	}

	bool applyMoveFleet(WorldData& world, const MoveFleetToSystem& order)
	{
		Fleet* fleet = world.fleetById(order.fleetId);
		if (fleet == nullptr || world.systemById(order.systemId) == nullptr)
		{
			return false;
		}

		if (fleet->motion.progress > 0.0f)
		{
			return false;
		}

		if (fleet->systemId == order.systemId)
		{
			return false;
		}

		const std::vector<long long> path = shortestLanePath(world, fleet->systemId, order.systemId);
		if (path.size() < 2)
		{
			return false;
		}

		fleet->motion.route = path;
		fleet->motion.hop = 0;
		fleet->motion.progress = 0.0f;
		fleet->motion.direction = 1;
		return true;
	}

}

	GameCommand GameCommand::moveFleetToSystem(
		long long fleetId,
		long long systemId,
		CommandSource source)
	{
		GameCommand command;
		command.source = source;
		command.payload = MoveFleetToSystem{ .fleetId = fleetId, .systemId = systemId };
		return command;
	}

	bool execute(WorldData& world, const GameCommand& command)
	{
		return std::visit(
			[&world](const auto& payload)
			{
				using Payload = std::decay_t<decltype(payload)>;
				if constexpr (std::is_same_v<Payload, MoveFleetToSystem>)
				{
					return applyMoveFleet(world, payload);
				}
			},
			command.payload);
	}

}
