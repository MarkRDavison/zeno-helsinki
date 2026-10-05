#pragma once

#include <helsinki/Engine/ECS/Component.hpp>

namespace tower
{
	class PathFollowComponent : public hl::Component
	{
	public:
		int fromIndex = 0;
		float t = 0.0f;
		float speed = 1.5f;
	};
}
