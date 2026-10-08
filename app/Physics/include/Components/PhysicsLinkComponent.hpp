#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/Physics/Types.hpp>
#include <helsinki/System/glm.hpp>
#include <string>

namespace phys
{
	class PhysicsLinkComponent : public hl::Component
	{
	public:
		hl::physics::BodyId body;
		hl::physics::CharacterId character;
		glm::vec3 visualHalfExtents{0.5f};
		glm::vec4 color{1.f, 1.f, 1.f, 1.f};
		std::string materialName;
		bool isCharacter = false;
	};
}
