#pragma once

#include <helsinki/System/glm.hpp>
#include <cstdint>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

namespace hl
{

	class InputManager
	{
	public:

		glm::vec2 getMousePosition() const;
		bool hasMouseMoved() const;
		glm::vec2 getWindowSize() const;
		glm::vec2 getFramebufferSize() const;
		bool isKeyDown(int _key) const;
		bool isButtonDown(int _button) const;

		bool isKeyReleased(int _key) const;
		bool isButtonReleased(int _button) const;

		void setWindow(GLFWwindow* _window);
		void pushChar(uint32_t codepoint);
		const std::vector<uint32_t>& charsThisFrame() const { return _charsThisFrame; }

		void updateEndOfFrame();
	private:
		GLFWwindow* m_Window;
		mutable std::unordered_map<int, bool> _wasKeyDown;
		mutable std::unordered_map<int, bool> _wasButtonDown;
		mutable glm::vec2 _lastMousePosition;
		std::vector<uint32_t> _charsThisFrame;
	};

}