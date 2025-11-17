#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

	class ScrollEvent : public Event
	{
	public:
		ScrollEvent(int x, int y) : _x(x), _y(y) {}

		int getX() const { return _x; }
		int getY() const { return _y; }

		DEFINE_EVENT_TYPE(ScrollEvent)

	private:
		int _x;
		int _y;
	};

}