#include <World/LaneMotion.hpp>
#include <algorithm>

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

}
