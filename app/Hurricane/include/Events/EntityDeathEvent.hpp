#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	enum class DeathType
	{
		DROP,
		NO_DROP
	};

	class EntityDeathEvent : public hl::Event
	{
	public:
		EntityDeathEvent(int id, DeathType type) : _id(id), _type(type) {}

		int getId() const { return _id; }
		DeathType getDeathType() const { return _type; }

		DEFINE_EVENT_TYPE(EntityDeathEvent)

	private:
		int _id;
		DeathType _type;
	};

}