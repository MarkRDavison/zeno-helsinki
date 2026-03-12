#include <helsinki/Engine/Input/InputManager.hpp>
#include <GLFW/glfw3.h>

namespace hl
{
	glm::vec2 InputManager::getMousePosition() const
	{
		return {};
	}
	glm::vec2 InputManager::getWindowSize() const
	{
		return {};
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