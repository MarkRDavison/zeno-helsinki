#include <World/PickFleet.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <World/LaneMotion.hpp>
#include <limits>

namespace slr
{

	glm::vec2 fleetParkedOffset(const WorldData& world, const Fleet& fleet)
	{
		if (world.empires.empty() || fleet.empireId == world.empires.front().id)
		{
			return GalaxyFleetOffset;
		}

		return -GalaxyFleetOffset;
	}

	std::optional<glm::vec2> fleetMarkerPosition(const WorldData& world, const Fleet& fleet)
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

		return system->position + fleetParkedOffset(world, fleet);
	}

	std::optional<long long> pickFleet(const WorldData& world, glm::vec2 worldPos, float radius)
	{
		std::optional<long long> bestId;
		float bestDist = std::numeric_limits<float>::infinity();

		for (const auto& fleet : world.fleets)
		{
			if (fleetShipCount(fleet) == 0)
			{
				continue;
			}

			const auto marker = fleetMarkerPosition(world, fleet);
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
