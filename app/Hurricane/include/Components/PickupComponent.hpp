#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>

namespace hur
{
	class PickupComponent : public hl::Component
	{
	public:
		std::string pickupId;
	};
}
