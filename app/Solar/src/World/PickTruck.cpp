#include <World/PickTruck.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <World/LaneMotion.hpp>
#include <limits>

namespace slr
{

	LaneMotion truckMotionForMarker(const WorldData& world, const Truck& truck)
	{
		const Convoy* convoy = world.convoyById(truck.convoyId);
		if (convoy == nullptr || !convoyIsUnderway(*convoy))
		{
			return truck.motion;
		}

		LaneMotion motion = convoy->motion;
		motion.speed = truck.motion.speed;
		return motion;
	}

	bool truckSharesStandingEscortFleetHop(
		const WorldData& world,
		const Truck& truck,
		const LaneMotion& motion)
	{
		const Convoy* convoy = world.convoyById(truck.convoyId);
		if (convoy == nullptr || !convoyIsStanding(*convoy))
		{
			return false;
		}

		if (convoyIsUnderway(*convoy))
		{
			return true;
		}

		const Fleet* fleet = world.fleetById(convoy->escortFleetId);
		return fleet != nullptr && fleet->motion.route.size() >= 2 &&
			motion.route == fleet->motion.route && motion.hop == fleet->motion.hop;
	}

	std::optional<glm::vec2> truckMarkerPosition(const WorldData& world, const Truck& truck)
	{
		const LaneMotion motion = truckMotionForMarker(world, truck);
		const auto onLane = laneWorldPosition(world, motion);
		if (onLane.has_value())
		{
			if (truckSharesStandingEscortFleetHop(world, truck, motion) &&
				motion.progress > 0.0f && motion.progress < 1.0f)
			{
				long long fromId = 0;
				long long toId = 0;
				if (!hopEndpoints(motion, fromId, toId))
				{
					return onLane;
				}

				const StarSystem* from = world.systemById(fromId);
				const StarSystem* to = world.systemById(toId);
				if (from == nullptr || to == nullptr)
				{
					return onLane;
				}

				const glm::vec2 delta = to->position - from->position;
				const float length = glm::length(delta);
				if (length <= 0.0f)
				{
					return onLane;
				}

				const glm::vec2 along = delta / length;
				const float trail =
					GalaxyFleetSize * 0.5f + GalaxyTruckSize * 0.5f + 4.0f;
				return *onLane - along * trail;
			}

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
