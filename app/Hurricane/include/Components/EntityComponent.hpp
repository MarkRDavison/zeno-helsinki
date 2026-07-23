#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>

namespace hur
{

	class EntityComponent : public hl::Component
	{
	public:
		std::string SpriteName;
	private:

	};

}