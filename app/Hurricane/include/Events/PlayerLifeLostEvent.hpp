#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class PlayerLifeLostEvent : public hl::Event
	{
	public:
		DEFINE_EVENT_TYPE(PlayerLifeLostEvent)
	};

}