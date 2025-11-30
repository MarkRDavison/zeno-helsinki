#include <helsinki/System/Infrastructure/Camera2D.hpp>

namespace hl
{


	glm::mat4 Camera2D::getViewMatrix() const
	{
		return glm::mat4(1.0f);
	}
	glm::mat4 Camera2D::getProjectionMatrix() const
	{
		glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, 0.0f, 1.0f);
		proj[1][1] *= -1.0f; // flip Y to match Vulkan NDC
		return proj;
	}

}