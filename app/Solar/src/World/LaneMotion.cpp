#include <World/LaneMotion.hpp>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <unordered_map>
#include <utility>

namespace slr
{

	bool hopEndpoints(const LaneMotion& motion, long long& fromId, long long& toId)
	{
		if (motion.route.size() < 2)
		{
			return false;
		}

		const int toIndex = motion.hop + motion.direction;
		if (motion.hop < 0 || toIndex < 0)
		{
			return false;
		}

		if (motion.hop >= static_cast<int>(motion.route.size()) ||
			toIndex >= static_cast<int>(motion.route.size()))
		{
			return false;
		}

		fromId = motion.route[static_cast<size_t>(motion.hop)];
		toId = motion.route[static_cast<size_t>(toIndex)];
		return true;
	}

	std::optional<glm::vec2> laneWorldPosition(const WorldData& world, const LaneMotion& motion)
	{
		long long fromId = 0;
		long long toId = 0;
		if (!hopEndpoints(motion, fromId, toId))
		{
			return std::nullopt;
		}

		const StarSystem* from = world.systemById(fromId);
		const StarSystem* to = world.systemById(toId);
		if (from == nullptr || to == nullptr)
		{
			return std::nullopt;
		}

		return glm::mix(from->position, to->position, motion.progress);
	}

	bool advanceAlongLane(WorldData& world, LaneMotion& motion)
	{
		long long fromId = 0;
		long long toId = 0;
		if (!hopEndpoints(motion, fromId, toId))
		{
			return false;
		}

		const StarSystem* from = world.systemById(fromId);
		const StarSystem* to = world.systemById(toId);
		if (from == nullptr || to == nullptr)
		{
			return false;
		}

		if (!hasHyperlane(world, fromId, toId))
		{
			return false;
		}

		const float length = glm::length(to->position - from->position);
		if (length <= 0.0f || motion.speed <= 0.0f)
		{
			return false;
		}

		motion.progress = std::min(1.0f, motion.progress + motion.speed / length);
		if (motion.progress < 1.0f)
		{
			return false;
		}

		motion.hop += motion.direction;
		motion.progress = 0.0f;
		return true;
	}

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

	long long civilianEnterHopCost(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId,
		bool seesEscort)
	{
		if (systemHasParkedHostileFleet(world, systemId, viewerEmpireId) ||
			systemClaimBlocksCivilian(world, systemId, viewerEmpireId))
		{
			return -1;
		}

		const bool extra =
			systemHasHazardHeat(world, systemId, viewerEmpireId) ||
			systemIsAdjacentToParkedHostileFleet(world, systemId, viewerEmpireId) ||
			systemIsAdjacentToHazardHeat(world, systemId, viewerEmpireId);
		if (extra && !seesEscort)
		{
			return 2;
		}

		return 1;
	}

	std::vector<long long> civilianLanePath(
		const WorldData& world,
		long long fromId,
		long long toId,
		long long viewerEmpireId,
		bool seesEscort)
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

		constexpr long long unreachable = std::numeric_limits<long long>::max();
		std::unordered_map<long long, long long> bestCost;
		std::unordered_map<long long, long long> parent;
		std::priority_queue<
			std::pair<long long, long long>,
			std::vector<std::pair<long long, long long>>,
			std::greater<std::pair<long long, long long>>>
			frontier;
		bestCost[fromId] = 0;
		parent[fromId] = fromId;
		frontier.push({ 0, fromId });

		while (!frontier.empty())
		{
			const auto [cost, current] = frontier.top();
			frontier.pop();
			if (cost > bestCost[current])
			{
				continue;
			}

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
				const long long hop = civilianEnterHopCost(world, next, viewerEmpireId, seesEscort);
				if (hop < 0)
				{
					continue;
				}

				const long long nextCost = cost + hop;
				const auto existing = bestCost.find(next);
				if (existing != bestCost.end() && existing->second <= nextCost)
				{
					continue;
				}

				bestCost[next] = nextCost;
				parent[next] = current;
				frontier.push({ nextCost, next });
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

	long long civilianLanePathCost(
		const WorldData& world,
		long long fromId,
		long long toId,
		long long viewerEmpireId,
		bool seesEscort)
	{
		const std::vector<long long> path =
			civilianLanePath(world, fromId, toId, viewerEmpireId, seesEscort);
		if (path.empty())
		{
			return -1;
		}

		if (path.size() == 1)
		{
			return 0;
		}

		long long cost = 0;
		for (size_t i = 1; i < path.size(); ++i)
		{
			const long long hop = civilianEnterHopCost(world, path[i], viewerEmpireId, seesEscort);
			if (hop < 0)
			{
				return -1;
			}

			cost += hop;
		}

		return cost;
	}

	long long laneMotionRemainingTicks(const WorldData& world, const LaneMotion& motion)
	{
		if (motion.route.size() < 2 || motion.speed <= 0.0f)
		{
			return 0;
		}

		long long ticks = 0;
		int hop = motion.hop;
		float progress = motion.progress;
		while (true)
		{
			const int toIndex = hop + motion.direction;
			if (hop < 0 || toIndex < 0 ||
				hop >= static_cast<int>(motion.route.size()) ||
				toIndex >= static_cast<int>(motion.route.size()))
			{
				break;
			}

			const StarSystem* from = world.systemById(motion.route[static_cast<size_t>(hop)]);
			const StarSystem* to = world.systemById(motion.route[static_cast<size_t>(toIndex)]);
			if (from == nullptr || to == nullptr)
			{
				break;
			}

			const float length = glm::length(to->position - from->position);
			if (length <= 0.0f)
			{
				hop = toIndex;
				progress = 0.0f;
				continue;
			}

			const float remaining = (1.0f - progress) * length;
			ticks += static_cast<long long>(std::ceil(static_cast<double>(remaining / motion.speed)));
			hop = toIndex;
			progress = 0.0f;
		}

		return ticks;
	}

}
