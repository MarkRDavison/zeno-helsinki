#pragma once

#include <helsinki/System/glm.hpp>
#include <optional>

namespace hl
{
	class Camera;
}

namespace tower
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
	};

	Ray rayFromCameraMouse(
		const hl::Camera& camera,
		glm::vec2 mousePosition,
		glm::vec2 windowSize);

	std::optional<glm::vec3> intersectGroundY0(const Ray& ray);

	glm::vec3 snapToTileCenter(const glm::vec3& hit, float markerY);
}
