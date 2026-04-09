#pragma once

#include <helsinki/System/glm.hpp>
#include <unordered_map>

struct GLFWwindow;

namespace hl
{

	class InputManager
	{
	public:

		glm::vec2 getMousePosition() const;
		bool hasMouseMoved() const;
		glm::vec2 getWindowSize() const;
		bool isKeyDown(int _key) const;
		bool isButtonDown(int _button) const;

		bool isKeyReleased(int _key) const;
		bool isButtonReleased(int _button) const;

		void setWindow(GLFWwindow* _window);

		void updateEndOfFrame();
	private:
		GLFWwindow* m_Window;
		mutable std::unordered_map<int, bool> _wasKeyDown;
		mutable std::unordered_map<int, bool> _wasButtonDown;
		mutable glm::vec2 _lastMousePosition;
	};

}