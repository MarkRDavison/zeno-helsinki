#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <string>

namespace tower
{
	class TowerComponent : public hl::Component
	{
	public:
		int x = 0;
		int z = 0;
		std::string defId;
		float fireCooldownRemaining = 0.0f;
	};
}
