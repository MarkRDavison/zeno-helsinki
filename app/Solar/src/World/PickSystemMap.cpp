#include <World/PickSystemMap.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace slr
{
namespace
{
	constexpr float FleetPickRadius = 12.0f;
	constexpr float TruckPickHalfWidth = 10.0f;
	constexpr float TruckPickHalfHeight = 6.0f;

	bool hitFleet(ImVec2 moverScreen, ImVec2 mouse)
	{
		const float dx = mouse.x - moverScreen.x;
		const float dy = mouse.y - moverScreen.y;
		return (dx * dx + dy * dy) <= FleetPickRadius * FleetPickRadius;
	}

	bool hitTruck(ImVec2 moverScreen, ImVec2 mouse)
	{
		return mouse.x >= moverScreen.x - TruckPickHalfWidth &&
			mouse.x <= moverScreen.x + TruckPickHalfWidth &&
			mouse.y >= moverScreen.y - TruckPickHalfHeight &&
			mouse.y <= moverScreen.y + TruckPickHalfHeight;
	}

	float hitDistance(ImVec2 moverScreen, ImVec2 mouse)
	{
		const float dx = mouse.x - moverScreen.x;
		const float dy = mouse.y - moverScreen.y;
		return std::sqrt(dx * dx + dy * dy);
	}
}

	SystemMapPick pickSystemMap(
		const WorldData& /*world*/,
		long long /*systemId*/,
		const SystemLayout& layout,
		const SystemMapCamera& camera,
		const std::vector<SystemMapMoverDraw>& movers,
		ImVec2 mouseScreen)
	{
		SystemMapPick bestFleet{ SystemMapPickKind::None, 0 };
		float bestFleetDist = std::numeric_limits<float>::infinity();
		SystemMapPick bestTruck{ SystemMapPickKind::None, 0 };
		float bestTruckDist = std::numeric_limits<float>::infinity();

		for (const auto& mover : movers)
		{
			const ImVec2 screen = camera.systemSpaceToScreen(mover.systemPosition);
			if (mover.ref.kind == SystemMapMoverKind::Fleet)
			{
				if (!hitFleet(screen, mouseScreen))
				{
					continue;
				}

				const float dist = hitDistance(screen, mouseScreen);
				if (dist < bestFleetDist ||
					(dist == bestFleetDist && mover.ref.id < bestFleet.id))
				{
					bestFleetDist = dist;
					bestFleet = { SystemMapPickKind::Fleet, mover.ref.id };
				}
			}
			else
			{
				if (!hitTruck(screen, mouseScreen))
				{
					continue;
				}

				const float dist = hitDistance(screen, mouseScreen);
				if (dist < bestTruckDist ||
					(dist == bestTruckDist && mover.ref.id < bestTruck.id))
				{
					bestTruckDist = dist;
					bestTruck = { SystemMapPickKind::Truck, mover.ref.id };
				}
			}
		}

		if (bestFleet.kind == SystemMapPickKind::Fleet)
		{
			return bestFleet;
		}

		if (bestTruck.kind == SystemMapPickKind::Truck)
		{
			return bestTruck;
		}

		const glm::vec2 systemPos = camera.screenToSystemSpace(mouseScreen);
		const SystemPlace* place = pickSystemPlace(layout, systemPos);
		if (place != nullptr)
		{
			return { SystemMapPickKind::Place, place->id };
		}

		return {};
	}

}
