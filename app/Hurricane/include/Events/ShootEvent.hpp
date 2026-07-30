#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class ShootEvent : public hl::Event
	{
	public:
		ShootEvent(int shooterId) : _shooterId(shooterId) {}

		int getShooterId() const { return _shooterId; }

		DEFINE_EVENT_TYPE(ShootEvent)

	private:
		int _shooterId;
	};

}