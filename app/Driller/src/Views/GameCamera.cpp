#include <Views/GameCamera.hpp>
#include <algorithm>

namespace drl
{

	glm::mat4 GameCamera::getViewMatrix() const
	{
		glm::mat4 view(1.0f);
		view = glm::scale(view, glm::vec3(_zoom, _zoom, 1.0f));
		view = glm::translate(view, glm::vec3(-_pan, 0.0f));
		return view;
	}

	glm::mat4 GameCamera::getProjectionMatrix() const
	{
		glm::mat4 proj = glm::ortho(0.0f, (float)_width, 0.0f, (float)_height, 0.0f, 1.0f);
		proj[1][1] *= -1.0f;
		return proj;
	}

	void GameCamera::notifyFramebufferChangeSize(uint32_t width, uint32_t height)
	{
		_width = width;
		_height = height;
	}

	glm::vec2 GameCamera::screenToWorld(glm::vec2 screen) const
	{
		return screen / _zoom + _pan;
	}

	void GameCamera::zoomAt(glm::vec2 screen, float factor)
	{
		const glm::vec2 before = screenToWorld(screen);
		_zoom = std::clamp(_zoom * factor, ZoomMin, ZoomMax);
		const glm::vec2 after = screenToWorld(screen);
		_pan += before - after;
	}

	void GameCamera::panByScreenDelta(glm::vec2 delta)
	{
		_pan -= delta / _zoom;
	}

}
