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
	};
}
