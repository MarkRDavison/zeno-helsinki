#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>

namespace slr
{

	std::optional<long long> pickStarSystem(
		const WorldData& world,
		glm::vec2 worldPos,
		float radius);

}
