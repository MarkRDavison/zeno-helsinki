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
	long long haulEnterHopCost(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId,
		bool seesEscort = false,
		bool isMilitary = false);
	std::vector<long long> haulLanePath(
		const WorldData& world,
		long long fromId,
		long long toId,
		long long viewerEmpireId,
		bool seesEscort = false,
		bool isMilitary = false);
	long long haulLanePathCost(
		const WorldData& world,
		long long fromId,
		long long toId,
		long long viewerEmpireId,
		bool seesEscort = false,
		bool isMilitary = false);
	long long civilianEnterHopCost(
		const WorldData& world,
		long long systemId,
		long long viewerEmpireId,
		bool seesEscort = false);
	std::vector<long long> civilianLanePath(
		const WorldData& world,
		long long fromId,
		long long toId,
		long long viewerEmpireId,
		bool seesEscort = false);
	long long civilianLanePathCost(
		const WorldData& world,
		long long fromId,
		long long toId,
		long long viewerEmpireId,
		bool seesEscort = false);
	long long laneMotionRemainingTicks(const WorldData& world, const LaneMotion& motion);

}
