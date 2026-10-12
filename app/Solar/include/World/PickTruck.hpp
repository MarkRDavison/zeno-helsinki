#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>

namespace slr
{

	std::optional<glm::vec2> truckMarkerPosition(const WorldData& world, const Truck& truck);
	std::optional<long long> pickTruck(const WorldData& world, glm::vec2 worldPos, float radius);

}
