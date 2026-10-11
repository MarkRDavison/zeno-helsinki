#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>

namespace slr
{

	std::optional<glm::vec2> fleetMarkerPosition(const WorldData& world, const Fleet& fleet, glm::vec2 offset);

	std::optional<long long> pickFleet(
		const WorldData& world,
		glm::vec2 worldPos,
		float radius,
		glm::vec2 markerOffset);

}
