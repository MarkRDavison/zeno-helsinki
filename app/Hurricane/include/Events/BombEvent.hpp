#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace hur
{
	class BombEvent : public hl::Event
	{
	public:
		BombEvent(int playerId) : _playerId(playerId) {}

		int getPlayerId() const { return _playerId; }

		DEFINE_EVENT_TYPE(BombEvent)

	private:
		int _playerId;
	};

}
