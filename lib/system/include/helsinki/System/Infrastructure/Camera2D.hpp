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
	private:

	};
}