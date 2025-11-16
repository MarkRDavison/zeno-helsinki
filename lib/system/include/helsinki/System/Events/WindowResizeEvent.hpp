#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int w, int h) : _width(w), _height(h) {}

		int GetWidth() const { return _width; }
		int GetHeight() const { return _height; }

		DEFINE_EVENT_TYPE(WindowResizeEvent)

	private:
		int _width;
		int _height;
	};

}