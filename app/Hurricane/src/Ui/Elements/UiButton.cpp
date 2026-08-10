#include <Ui/Elements/UiButton.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

namespace hur
{
	UiButton::UiButton(GLFWwindow* window) : _window(window)
	{
		_defaultColour = { 1.0f, 0.0f, 0.0f, 1.0f };
		_hoverColour = { 0.0f, 1.0f, 0.0f, 1.0f };
		_pressedColour = { 0.0f, 0.0f, 1.0f, 1.0f };
		_currentColour = _defaultColour;
	}

	void UiButton::update(float delta)
	{
        double mouseX;
        double mouseY;

        // TODO: REPLACE WITH INPUT MANAGER
        glfwGetCursorPos(_window, &mouseX, &mouseY);

        const bool hovered =
            mouseX >= calculatedRect.position.x &&
            mouseX <= calculatedRect.position.x + calculatedRect.size.x &&
            mouseY >= calculatedRect.position.y &&
            mouseY <= calculatedRect.position.y + calculatedRect.size.y;

        const bool down =
            glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (hovered && down)
        {
            _currentColour = _pressedColour;
        }
        else if (hovered)
        {
            _currentColour = _hoverColour;
        }
        else
        {
            _currentColour = _defaultColour;
        }

        if (hovered && down && !_wasPressedLastFrame && onClick)
        {
            onClick();
        }

        _wasPressedLastFrame = down;
	}

	void UiButton::draw(hl::UiRoot& root, glm::vec2 screenSize)
	{
		root.addQuad(calculatedRect, _currentColour);
	}

}