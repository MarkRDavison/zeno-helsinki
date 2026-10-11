#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>

namespace slr
{

	bool hopEndpoints(const LaneMotion& motion, long long& fromId, long long& toId);
	std::optional<glm::vec2> laneWorldPosition(const WorldData& world, const LaneMotion& motion);
	bool advanceAlongLane(WorldData& world, LaneMotion& motion);

}
