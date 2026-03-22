#pragma once

#include <helsinki/System/Infrastructure/BaseCamera.hpp>

namespace hl
{
	class Camera2D : public BaseCamera
	{
	public:
		~Camera2D() override = default;
		glm::mat4 getViewMatrix() const override;
		glm::mat4 getProjectionMatrix() const override;

		void notifyFramebufferChangeSize(uint32_t width, uint32_t height) override;

	private:
		uint32_t _width{ 0 };
		uint32_t _height{ 0 };
	};
}