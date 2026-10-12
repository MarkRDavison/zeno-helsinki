#include <World/PickTruck.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <World/LaneMotion.hpp>
#include <limits>

namespace slr
{

	std::optional<glm::vec2> truckMarkerPosition(const WorldData& world, const Truck& truck)
	{
		const auto onLane = laneWorldPosition(world, truck.motion);
		if (onLane.has_value())
		{
			return onLane;
		}

		const StarSystem* at = world.systemById(truckSystemId(truck));
		if (at == nullptr)
		{
			return std::nullopt;
		}

		return at->position + GalaxyTruckOffset;
	}

	std::optional<long long> pickTruck(const WorldData& world, glm::vec2 worldPos, float radius)
	{
		std::optional<long long> bestId;
		float bestDist = std::numeric_limits<float>::infinity();

		for (const auto& truck : world.trucks)
		{
			const auto marker = truckMarkerPosition(world, truck);
			if (!marker.has_value())
			{
				continue;
			}

			const float dist = glm::distance(*marker, worldPos);
			if (dist > radius)
			{
				continue;
			}

			if (dist < bestDist || (dist == bestDist && bestId.has_value() && truck.id < *bestId))
			{
				bestDist = dist;
				bestId = truck.id;
			}
		}

		return bestId;
	}

}
