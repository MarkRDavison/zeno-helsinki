#include <World/LaneMotion.hpp>
#include <algorithm>
#include <map>
#include <queue>
#include <unordered_map>

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

}
