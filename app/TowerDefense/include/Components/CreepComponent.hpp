#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class CreepComponent : public hl::Component
	{
	public:
		const char* material = nullptr;
	};
}
