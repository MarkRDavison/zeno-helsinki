#pragma once

#include <helsinki/System/Events/Event.hpp>
#include <cstdint>

namespace hl
{

	class CharEvent : public Event
	{
	public:
		explicit CharEvent(uint32_t codepoint) : _codepoint(codepoint) {}

		uint32_t codepoint() const { return _codepoint; }

		DEFINE_EVENT_TYPE(CharEvent)

	private:
		uint32_t _codepoint;
	};

}
