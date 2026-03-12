#pragma once

#include <helsinki/System/glm.hpp>

struct GLFWwindow;

namespace hl
{

	class InputManager
	{
	public:

		glm::vec2 getMousePosition() const;
		glm::vec2 getWindowSize() const;
		bool isKeyDown(int _key) const;
		bool isButtonDown(int _button) const;

		void setWindow(GLFWwindow* _window);
	private:
		GLFWwindow* m_Window;
	};

}