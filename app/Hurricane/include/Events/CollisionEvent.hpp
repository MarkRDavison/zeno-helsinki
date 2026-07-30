#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class CollisionEvent : public hl::Event
	{
	public:
		CollisionEvent(int entityAId, int entityBAId
			) :
			_entityAId(entityAId),
			_entityBId(entityBAId)
		{}

		int getEntityAId() const { return _entityAId; }
		int getEntityBId() const { return _entityBId; }

		DEFINE_EVENT_TYPE(CollisionEvent)

	private:
		int _entityAId;
		int _entityBId;
	};

}