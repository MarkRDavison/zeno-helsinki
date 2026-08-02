#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class PlayerScoreEvent : public hl::Event
	{
	public:
		PlayerScoreEvent(int amount) : _amount(amount) {}

		int getAmount() const { return _amount; }

		DEFINE_EVENT_TYPE(PlayerScoreEvent)
	private:
		int _amount;

	};

}