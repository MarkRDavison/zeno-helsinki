#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hl
{

	class KeyPressEvent : public Event
	{
	public:
		KeyPressEvent(int keyCode) : _keyCode(keyCode) {}

		int GetKeyCode() const { return _keyCode; }

		DEFINE_EVENT_TYPE(KeyPressEvent)

	private:
		int _keyCode;
	};

	class KeyReleaseEvent: public Event
	{
	public:
		KeyReleaseEvent(int keyCode) : _keyCode(keyCode) {}

		int GetKeyCode() const { return _keyCode; }

		DEFINE_EVENT_TYPE(KeyReleaseEvent)

	private:
		int _keyCode;
	};

}