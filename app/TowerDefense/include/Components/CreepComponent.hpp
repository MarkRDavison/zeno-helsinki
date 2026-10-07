#pragma once

#include <Services/WeaponCatalog.hpp>
#include <helsinki/Engine/ECS/Component.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace tower
{
	class CreepComponent : public hl::Component
	{
	public:
		std::unordered_map<std::string, float> resist;
		float baseHealth = 1.0f;
		float baseSpeed = 1.0f;
		float scale = 0.4f;
		float range = 0.0f;
		std::vector<WeaponSlot> slots;
		std::vector<float> slotCooldown;
	};
}
