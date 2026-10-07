#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <string>

namespace tower
{
	class StatusRingComponent : public hl::Component
	{
	public:
		int creepId = 0;
		std::string categoryId;
		std::string materialName;
	};
}
