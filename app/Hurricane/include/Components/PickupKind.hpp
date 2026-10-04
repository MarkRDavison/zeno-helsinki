#pragma once

#include <HurricaneConstants.hpp>
#include <string_view>

namespace hur
{
	enum class PickupKind
	{
		None,
		Bolt,
		Bomb,
		Speed,
		Shield
	};

	inline PickupKind pickupKindFromId(std::string_view pickupId)
	{
		if (pickupId == "powerupBlue_bolt")
		{
			return PickupKind::Bolt;
		}

		if (pickupId == "powerupBlue_star")
		{
			return PickupKind::Bomb;
		}

		if (pickupId == PickupIdSpeed)
		{
			return PickupKind::Speed;
		}

		if (pickupId == PickupIdShield)
		{
			return PickupKind::Shield;
		}

		return PickupKind::None;
	}
}
