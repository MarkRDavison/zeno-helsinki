#pragma once

#include <helsinki/System/Infrastructure/BaseCamera.hpp>
#include <helsinki/System/glm.hpp>

namespace slr
{

	class GalaxyCamera : public hl::BaseCamera
	{
	public:
		glm::mat4 getViewMatrix() const override;
		glm::mat4 getProjectionMatrix() const override;
		void notifyFramebufferChangeSize(uint32_t width, uint32_t height) override;

		void setOrigin(const glm::vec2& origin);
		const glm::vec2& origin() const { return _origin; }

		glm::vec2 worldFromScreen(const glm::vec2& screen) const;
		void panByScreenDelta(const glm::vec2& screenDelta);
		void zoomAtScreen(const glm::vec2& screen, int scrollY);

		uint32_t framebufferWidth() const { return _width; }
		uint32_t framebufferHeight() const { return _height; }

	private:
		float halfWidth() const;
		float halfHeight() const;

		glm::vec2 _origin{ 0.0f };
		float _zoom{ 1.0f };
		uint32_t _width{ 0 };
		uint32_t _height{ 0 };
	};

}
