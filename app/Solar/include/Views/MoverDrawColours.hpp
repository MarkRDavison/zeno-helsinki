#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <cstdint>

namespace slr
{

	bool isPlayerFleet(const WorldData& world, const Fleet& fleet);

	glm::vec4 fleetDrawColour(const WorldData& world, const Fleet& fleet, bool selected);
	glm::vec4 truckDrawColour(const WorldData& world, const Truck& truck, bool selected);

	std::uint32_t vec4ToImU32(const glm::vec4& colour);

}
