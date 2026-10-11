#include <World/PickFleet.hpp>
#include <World/LaneMotion.hpp>
#include <limits>

namespace slr
{

	std::optional<glm::vec2> fleetMarkerPosition(const WorldData& world, const Fleet& fleet, glm::vec2 offset)
	{
		const auto onLane = laneWorldPosition(world, fleet.motion);
		if (onLane.has_value())
		{
			return onLane;
		}

		const StarSystem* system = world.systemById(fleet.systemId);
		if (system == nullptr)
		{
			return std::nullopt;
		}

		return system->position + offset;
	}

	std::optional<long long> pickFleet(
		const WorldData& world,
		glm::vec2 worldPos,
		float radius,
		glm::vec2 markerOffset)
	{
		std::optional<long long> bestId;
		float bestDist = std::numeric_limits<float>::infinity();

		for (const auto& fleet : world.fleets)
		{
			const auto marker = fleetMarkerPosition(world, fleet, markerOffset);
			if (!marker.has_value())
			{
				continue;
			}

			const float dist = glm::distance(*marker, worldPos);
			if (dist > radius)
			{
				continue;
			}

			if (dist < bestDist || (dist == bestDist && bestId.has_value() && fleet.id < *bestId))
			{
				bestDist = dist;
				bestId = fleet.id;
			}
		}

		return bestId;
	}

}
