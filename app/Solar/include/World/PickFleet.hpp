#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>

namespace slr
{

	glm::vec2 fleetParkedOffset(const WorldData& world, const Fleet& fleet);
	std::optional<glm::vec2> fleetMarkerPosition(const WorldData& world, const Fleet& fleet);
	std::optional<long long> pickFleet(const WorldData& world, glm::vec2 worldPos, float radius);

}
