#pragma once

#include <Components/PickupKind.hpp>
#include <HurricaneConstants.hpp>
#include <WeaponCatalog.hpp>
#include <helsinki/Engine/ECS/Component.hpp>

namespace hur
{
	class PlayerLoadoutComponent : public hl::Component
	{
	public:
		int bombs{ 0 };
		float speedBoostRemaining{ 0.0f };
		int shieldLayers{ 0 };
	};

	inline void applyPickup(
		PlayerLoadoutComponent& loadout,
		WeaponComponent* weapon,
		PickupKind kind)
	{
		switch (kind)
		{
		case PickupKind::Bolt:
			if (weapon != nullptr && weapon->Type != WeaponTypeDualLaser)
			{
				applyWeapon(*weapon, WeaponTypeDualLaser);
			}
			break;
		case PickupKind::Bomb:
			++loadout.bombs;
			break;
		case PickupKind::Speed:
			loadout.speedBoostRemaining = SpeedBoostDuration;
			break;
		case PickupKind::Shield:
			loadout.shieldLayers = PlayerShieldMaxLayers;
			break;
		case PickupKind::None:
		default:
			break;
		}
	}
}
