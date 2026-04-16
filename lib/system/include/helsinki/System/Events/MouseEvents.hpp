#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

	class MouseButtonPressEvent : public Event
	{
	public:
		MouseButtonPressEvent(int buttonCode) : _buttonCode(buttonCode) {}

		int GetKeyCode() const { return _buttonCode; }

		DEFINE_EVENT_TYPE(MouseButtonPressEvent)
	private:
		int _buttonCode;
	};

	class MouseButtonReleaseEvent : public Event
	{
	public:
		MouseButtonReleaseEvent(int buttonCode) : _buttonCode(buttonCode) {}

		int GetKeyCode() const { return _buttonCode; }

		DEFINE_EVENT_TYPE(MouseButtonReleaseEvent)
	private:
		int _buttonCode;
	};

	class MousePositionEvent : public Event
	{
	public:
		MousePositionEvent(int x, int y) : _x(x), _y(y) {}
		
		int getX() const { return _x; }
		int getY() const { return _y; }

		DEFINE_EVENT_TYPE(MousePositionEvent)

	private:
		int _x;
		int _y;
	};

}