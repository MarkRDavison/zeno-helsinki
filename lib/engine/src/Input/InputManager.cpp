#include <helsinki/Engine/Input/InputManager.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace hl
{
	// TODO: Overload relative to camera???
	glm::vec2 InputManager::getMousePosition() const
	{
		double x, y;
		glfwGetCursorPos(m_Window, &x, &y);

		return glm::vec2(x, y);
	}
	glm::vec2 InputManager::getWindowSize() const
	{
		int w, h;
		glfwGetWindowSize(m_Window, &w, &h);
		return glm::vec2(w, h);
	}
	bool InputManager::isKeyDown(int _key) const
	{
		return glfwGetKey(m_Window, _key) == GLFW_PRESS;
	}

	bool InputManager::isButtonDown(int _button) const
	{
		return glfwGetMouseButton(m_Window, _button) == GLFW_PRESS;
	}
	void InputManager::setWindow(GLFWwindow* _window)
	{
		m_Window = _window;
	}
}