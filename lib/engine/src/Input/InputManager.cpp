#include <helsinki/Engine/Input/InputManager.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

namespace hl
{
	// TODO: Overload relative to camera???
	glm::vec2 InputManager::getMousePosition() const
	{
		double x, y;
		glfwGetCursorPos(m_Window, &x, &y);

		return glm::vec2(x, y);
	}
	bool InputManager::hasMouseMoved() const
	{
		return getMousePosition() != _lastMousePosition;
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

	bool InputManager::isKeyReleased(int _key)const
	{
		if (isKeyDown(_key))
		{
			return false;
		}

		if (_wasKeyDown.contains(_key))
		{
			return _wasKeyDown.at(_key);
		}
		else
		{
			_wasKeyDown.insert({ _key, false });
		}

		return false;
	}
	bool InputManager::isButtonReleased(int _button) const
	{
		if (isButtonDown(_button))
		{
			return false;
		}

		if (_wasButtonDown.contains(_button))
		{
			return _wasButtonDown.at(_button);
		}
		else
		{
			_wasButtonDown.insert({ _button, false });
		}

		return false;
	}

	void InputManager::setWindow(GLFWwindow* _window)
	{
		m_Window = _window;
	}
	void InputManager::updateEndOfFrame()
	{
		_lastMousePosition = getMousePosition();

		for (auto& [button, state] : _wasButtonDown)
		{
			state = glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
		}

		for (auto& [key, state] : _wasKeyDown)
		{
			state = glfwGetKey(m_Window, key) == GLFW_PRESS;
		}
	}
}