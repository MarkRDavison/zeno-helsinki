#include <Views/GalaxyCamera.hpp>
#include <algorithm>

namespace slr
{
namespace
{
	constexpr float MinZoom = 0.25f;
	constexpr float MaxZoom = 4.0f;
	constexpr float ZoomStep = 1.1f;
}

	glm::mat4 GalaxyCamera::getViewMatrix() const
	{
		return glm::mat4(1.0f);
	}

	glm::mat4 GalaxyCamera::getProjectionMatrix() const
	{
		if (_width == 0 || _height == 0)
		{
			return glm::mat4(1.0f);
		}

		const float hw = halfWidth();
		const float hh = halfHeight();
		glm::mat4 proj = glm::ortho(
			_origin.x - hw,
			_origin.x + hw,
			_origin.y - hh,
			_origin.y + hh,
			0.0f,
			1.0f);
		proj[1][1] *= -1.0f;
		return proj;
	}

	void GalaxyCamera::notifyFramebufferChangeSize(uint32_t width, uint32_t height)
	{
		_width = width;
		_height = height;
	}

	void GalaxyCamera::setOrigin(const glm::vec2& origin)
	{
		_origin = origin;
	}

	glm::vec2 GalaxyCamera::worldFromScreen(const glm::vec2& screen) const
	{
		if (_width == 0 || _height == 0)
		{
			return _origin;
		}

		const float hw = halfWidth();
		const float hh = halfHeight();
		const float nx = screen.x / static_cast<float>(_width);
		const float ny = screen.y / static_cast<float>(_height);
		return {
			_origin.x - hw + nx * 2.0f * hw,
			_origin.y - hh + ny * 2.0f * hh
		};
	}

	void GalaxyCamera::panByScreenDelta(const glm::vec2& screenDelta)
	{
		_origin -= screenDelta / _zoom;
	}

	void GalaxyCamera::zoomAtScreen(const glm::vec2& screen, int scrollY)
	{
		if (scrollY == 0)
		{
			return;
		}

		const glm::vec2 before = worldFromScreen(screen);
		if (scrollY > 0)
		{
			_zoom = std::min(MaxZoom, _zoom * ZoomStep);
		}
		else
		{
			_zoom = std::max(MinZoom, _zoom / ZoomStep);
		}

		const glm::vec2 after = worldFromScreen(screen);
		_origin += before - after;
	}

	float GalaxyCamera::halfWidth() const
	{
		return static_cast<float>(_width) / (2.0f * _zoom);
	}

	float GalaxyCamera::halfHeight() const
	{
		return static_cast<float>(_height) / (2.0f * _zoom);
	}

}
