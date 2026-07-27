#include <GameCamera.hpp>
#include <HurricaneConstants.hpp>

namespace hur
{

	glm::mat4x4 GameCamera::getViewMatrix() const
	{
		return glm::mat4(1.0f);
	}

	glm::mat4x4 GameCamera::getProjectionMatrix() const
	{
		glm::mat4 proj = glm::ortho(0.0f, (float)HurricaneConstants::Width, 0.0f, (float)HurricaneConstants::Height, 0.0f, 1.0f);
		proj[1][1] *= -1.0f; // flip Y to match Vulkan NDC
		return proj;
	}

	void GameCamera::notifyFramebufferChangeSize(uint32_t width, uint32_t height)
	{
		_width = width;
		_height = height;
	}
}