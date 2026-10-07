#pragma once

#include <Status.hpp>
#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class StatusListComponent : public hl::Component
	{
	public:
		std::vector<StatusInstance> instances;
		int nextSeq = 0;
	};
}
