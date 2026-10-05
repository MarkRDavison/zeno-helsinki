#include <GroundPick.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>
#include <cmath>

namespace tower
{
	Ray rayFromCameraMouse(
		const hl::Camera& camera,
		glm::vec2 mousePosition,
		glm::vec2 windowSize)
	{
		const float w = windowSize.x;
		const float h = windowSize.y;
		const float ndcX = (2.0f * mousePosition.x) / w - 1.0f;
		const float ndcY = (2.0f * mousePosition.y) / h - 1.0f;

		glm::mat4 proj = camera.getProjectionMatrix();
		proj[1][1] *= -1.0f;

		const glm::mat4 inv = glm::inverse(proj * camera.getViewMatrix());

		auto unproject = [&](float ndcZ)
		{
			glm::vec4 clip{ ndcX, ndcY, ndcZ, 1.0f };
			glm::vec4 world = inv * clip;
			world /= world.w;
			return glm::vec3(world);
		};

		const glm::vec3 nearPoint = unproject(0.0f);
		const glm::vec3 farPoint = unproject(1.0f);

		return Ray
		{
			.origin = nearPoint,
			.direction = glm::normalize(farPoint - nearPoint)
		};
	}

	std::optional<glm::vec3> intersectGroundY0(const Ray& ray)
	{
		constexpr float epsilon = 1e-6f;
		if (std::abs(ray.direction.y) < epsilon)
		{
			return std::nullopt;
		}

		const float t = -ray.origin.y / ray.direction.y;
		if (t <= 0.0f)
		{
			return std::nullopt;
		}

		return ray.origin + t * ray.direction;
	}

	glm::vec3 snapToTileCenter(const glm::vec3& hit, float markerY)
	{
		return glm::vec3(
			std::floor(hit.x) + 0.5f,
			markerY,
			std::floor(hit.z) + 0.5f);
	}
}
