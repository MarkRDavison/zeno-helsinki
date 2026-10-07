#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <string>

namespace tower
{
	class PathRibbonComponent : public hl::Component
	{
	public:
		std::string materialName;
	};
}
