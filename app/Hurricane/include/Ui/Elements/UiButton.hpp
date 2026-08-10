#pragma once

#include <Ui/UiElement.hpp>


struct GLFWwindow;

namespace hur
{

	class UiButton : public UiElement
	{
	public:
		UiButton(GLFWwindow*window);
		void update(float delta) override;
		void draw(hl::UiRoot& root, glm::vec2 screenSize) override;

		std::function<void()> onClick;
	private:
		GLFWwindow* _window;
		glm::vec4 _defaultColour;
		glm::vec4 _hoverColour;
		glm::vec4 _pressedColour;
		glm::vec4 _currentColour;

		bool _wasPressedLastFrame = false;
	};

}