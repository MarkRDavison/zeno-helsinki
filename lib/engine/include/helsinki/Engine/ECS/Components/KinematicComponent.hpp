#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>

namespace hl
{
	class KinematicComponent : public Component
	{
	public:
		glm::vec3 velocity = glm::vec3();
		glm::vec3 acceleration = glm::vec3();
	};
}