#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>

namespace tower
{
	class ProjectileComponent : public hl::Component
	{
	public:
		int targetId = 0;
		float speed = 0.0f;
		int damage = 1;
		float hitRadius = 0.35f;
		float y = 0.4f;
		glm::vec2 aimOffset{ 0.0f, 0.0f };
		glm::vec3 lastDest{ 0.0f, 0.0f, 0.0f };
	};
}
