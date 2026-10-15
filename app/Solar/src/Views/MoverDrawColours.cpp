#include <Views/MoverDrawColours.hpp>
#include <imgui.h>

namespace slr
{
namespace
{
	const glm::vec4 FleetColour{ 0.45f, 0.82f, 0.62f, 1.0f };
	const glm::vec4 SelectedFleetColour{ 0.7f, 0.95f, 0.8f, 1.0f };
	const glm::vec4 RivalFleetColour{ 0.85f, 0.38f, 0.4f, 1.0f };
	const glm::vec4 SelectedRivalFleetColour{ 1.0f, 0.55f, 0.55f, 1.0f };
	const glm::vec4 CivilianTruckColour{ 0.95f, 0.65f, 0.22f, 1.0f };
	const glm::vec4 SelectedCivilianTruckColour{ 1.0f, 0.85f, 0.45f, 1.0f };
	const glm::vec4 MilitaryTruckColour{ 0.85f, 0.15f, 0.72f, 1.0f };
	const glm::vec4 SelectedMilitaryTruckColour{ 1.0f, 0.45f, 0.92f, 1.0f };
	const glm::vec4 TenderColour{ 0.08f, 0.38f, 0.18f, 1.0f };
	const glm::vec4 SelectedTenderColour{ 0.22f, 0.62f, 0.32f, 1.0f };
}

	bool isPlayerFleet(const WorldData& world, const Fleet& fleet)
	{
		return world.empires.empty() || fleet.empireId == world.empires.front().id;
	}

	glm::vec4 fleetDrawColour(const WorldData& world, const Fleet& fleet, bool selected)
	{
		if (isPlayerFleet(world, fleet))
		{
			return selected ? SelectedFleetColour : FleetColour;
		}

		return selected ? SelectedRivalFleetColour : RivalFleetColour;
	}

	glm::vec4 truckDrawColour(const WorldData& world, const Truck& truck, bool selected)
	{
		if (truckHasRole(world, truck, HullRole::Tender))
		{
			return selected ? SelectedTenderColour : TenderColour;
		}

		if (truckHasRole(world, truck, HullRole::MilitaryCargo))
		{
			return selected ? SelectedMilitaryTruckColour : MilitaryTruckColour;
		}

		return selected ? SelectedCivilianTruckColour : CivilianTruckColour;
	}

	std::uint32_t vec4ToImU32(const glm::vec4& colour)
	{
		return static_cast<std::uint32_t>(IM_COL32(
			static_cast<int>(colour.r * 255.0f),
			static_cast<int>(colour.g * 255.0f),
			static_cast<int>(colour.b * 255.0f),
			static_cast<int>(colour.a * 255.0f)));
	}

}
