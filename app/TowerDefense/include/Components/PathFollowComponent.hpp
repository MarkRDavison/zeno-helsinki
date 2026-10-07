#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <string>

namespace tower
{
	class PathFollowComponent : public hl::Component
	{
	public:
		std::string pathName;
		int fromIndex = 0;
		float t = 0.0f;
		float baseSpeed = 1.5f;
		float speed = 1.5f;
	};
}
