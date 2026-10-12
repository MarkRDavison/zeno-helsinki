#pragma once

#include <World/SimClock.hpp>
#include <optional>

namespace slr
{

	enum class TimeSpeedControl
	{
		Pause,
		Play,
		Speed2,
		Speed4
	};

	struct TimeChromeResult
	{
		std::optional<bool> paused;
		std::optional<int> speed;
	};

	TimeSpeedControl activeTimeControl(bool paused, int speed);

	class TimeChrome
	{
	public:
		static constexpr float SpeedButtonWidth = 36.0f;

		TimeChromeResult draw(const SimClock& clock, long long tick) const;
	};

}
