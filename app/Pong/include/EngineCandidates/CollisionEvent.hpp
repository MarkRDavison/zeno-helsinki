#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace pong
{

	class CollisionEvent : public hl::Event
	{
	public:
		DEFINE_EVENT_TYPE(CollisionEvent)
	};

}