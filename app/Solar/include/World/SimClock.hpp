#pragma once

namespace slr
{

	class SimClock
	{
	public:
		static constexpr float TickPeriodSeconds = 0.5f;

		bool paused() const { return _paused; }
		int speed() const { return _speed; }

		void setPaused(bool paused);
		void togglePaused();
		void setSpeed(int speed);

		int takeTicks(float dt);

	private:
		bool _paused{ true };
		int _speed{ 1 };
		float _accum{ 0.0f };
	};

}
