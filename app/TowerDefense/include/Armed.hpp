#pragma once

#include <Components/HealthComponent.hpp>
#include <Services/WeaponCatalog.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>
#include <vector>

namespace tower
{
	inline bool canFireAtStalled(
		const std::vector<WeaponSlot>& slots,
		const hl::Entity* target)
	{
		return !slots.empty()
			&& target != nullptr
			&& target->GetComponent<HealthComponent>() != nullptr;
	}
}
