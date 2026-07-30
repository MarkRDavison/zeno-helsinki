#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class EntityDeathEvent : public hl::Event
	{
	public:
		EntityDeathEvent(int id) : _id(id) {}

		int getId() const { return _id; }

		DEFINE_EVENT_TYPE(EntityDeathEvent)

	private:
		int _id;
	};

}