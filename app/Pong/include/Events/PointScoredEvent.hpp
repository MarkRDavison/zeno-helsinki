#pragma once

#include <helsinki/System/Events/Event.hpp>

namespace pong
{

	class PointScoredEvent : public hl::Event
	{
	public:
		PointScoredEvent(int playerNumber) : _playerNumber(playerNumber) {}

		int GetPlayerNumber() const { return _playerNumber; }

		DEFINE_EVENT_TYPE(PointScoredEvent)

	private:
		int _playerNumber;
	};

}