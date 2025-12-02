#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{
	enum class CameraMovement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	class BaseCamera
	{
	public:
		virtual ~BaseCamera() {}
		virtual glm::mat4 getViewMatrix() const = 0;
		virtual glm::mat4 getProjectionMatrix() const = 0;

		virtual void notifyFramebufferChangeSize(uint32_t /*width*/, uint32_t /*height*/) {}

	protected:

	};

}