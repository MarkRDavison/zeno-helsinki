#pragma once

#include <helsinki/System/Events/Event.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>

namespace hur
{
	class ShootEvent : public hl::Event
	{
	public:
		ShootEvent(hl::Entity* shooter) : _shooter(shooter) {}

		hl::Entity* getShooter() const { return _shooter; }

		DEFINE_EVENT_TYPE(ShootEvent)

	private:
		hl::Entity* _shooter;
	};

}