#pragma once

#include <helsinki/System/Infrastructure/BaseCamera.hpp>

namespace drl
{

	class GameCamera : public hl::BaseCamera
	{
	public:
		static constexpr float ZoomMin = 0.25f;
		static constexpr float ZoomMax = 4.0f;
		static constexpr float ZoomStep = 1.1f;

		glm::mat4 getViewMatrix() const override;
		glm::mat4 getProjectionMatrix() const override;
		void notifyFramebufferChangeSize(uint32_t width, uint32_t height) override;

		glm::vec2 screenToWorld(glm::vec2 screen) const;
		void zoomAt(glm::vec2 screen, float factor);
		void panByScreenDelta(glm::vec2 delta);

		float zoom() const { return _zoom; }
		glm::vec2 pan() const { return _pan; }

	private:
		uint32_t _width{ 0 };
		uint32_t _height{ 0 };
		glm::vec2 _pan{ 0.0f, 0.0f };
		float _zoom{ 1.0f };
	};

}
