#pragma once

#include <helsinki/System/Infrastructure/BaseCamera.hpp>

namespace hur
{
	class UiCamera : public hl::BaseCamera
	{
	public:
		glm::mat4x4 getViewMatrix() const override;
		glm::mat4x4 getProjectionMatrix() const override;
		void notifyFramebufferChangeSize(uint32_t width, uint32_t height) override;

	protected:
		uint32_t _width{ 0 };
		uint32_t _height{ 0 };
	};

}