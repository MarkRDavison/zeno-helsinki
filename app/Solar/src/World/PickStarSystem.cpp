#include <World/PickStarSystem.hpp>
#include <limits>

namespace slr
{

	std::optional<long long> pickStarSystem(
		const WorldData& world,
		glm::vec2 worldPos,
		float radius)
	{
		std::optional<long long> bestId;
		float bestDist = std::numeric_limits<float>::infinity();

		for (const auto& system : world.systems)
		{
			const float dist = glm::distance(system.position, worldPos);
			if (dist > radius)
			{
				continue;
			}

			if (dist < bestDist || (dist == bestDist && bestId.has_value() && system.id < *bestId))
			{
				bestDist = dist;
				bestId = system.id;
			}
		}

		return bestId;
	}

}
