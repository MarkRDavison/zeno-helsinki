#include <helsinki/System/Infrastructure/Camera2D.hpp>

namespace hl
{


	glm::mat4 Camera2D::getViewMatrix() const
	{
		return glm::mat4(1.0f);
	}
	glm::mat4 Camera2D::getProjectionMatrix() const
	{
		return glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
	}

}