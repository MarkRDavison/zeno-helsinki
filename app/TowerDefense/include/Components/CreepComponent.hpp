#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <string>
#include <unordered_map>

namespace tower
{
	class CreepComponent : public hl::Component
	{
	public:
		std::unordered_map<std::string, float> resist;
		float baseHealth = 1.0f;
		float baseSpeed = 1.0f;
		float scale = 0.4f;
	};
}
