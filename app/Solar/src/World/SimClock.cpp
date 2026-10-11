#include <World/SimClock.hpp>

namespace slr
{

	void SimClock::setPaused(bool paused)
	{
		_paused = paused;
	}

	void SimClock::togglePaused()
	{
		_paused = !_paused;
	}

	void SimClock::setSpeed(int speed)
	{
		if (speed == 1 || speed == 2 || speed == 4)
		{
			_speed = speed;
		}
	}

	int SimClock::takeTicks(float dt)
	{
		if (_paused || dt <= 0.0f)
		{
			return 0;
		}

		_accum += dt * static_cast<float>(_speed);
		if (_accum < TickPeriodSeconds)
		{
			return 0;
		}

		_accum -= TickPeriodSeconds;
		return 1;
	}

}
