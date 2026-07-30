#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class EnemySpawnEvent : public hl::Event
	{
	public:
		DEFINE_EVENT_TYPE(EnemySpawnEvent)
	};

}