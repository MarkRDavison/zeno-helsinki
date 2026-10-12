#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>
#include <vector>

namespace slr
{

	bool hopEndpoints(const LaneMotion& motion, long long& fromId, long long& toId);
	std::optional<glm::vec2> laneWorldPosition(const WorldData& world, const LaneMotion& motion);
	bool advanceAlongLane(WorldData& world, LaneMotion& motion);
	std::vector<long long> shortestLanePath(const WorldData& world, long long fromId, long long toId);

}
