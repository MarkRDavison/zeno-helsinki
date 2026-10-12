#pragma once

#include <Views/SituationChrome.hpp>
#include <Views/TimeChrome.hpp>
#include <World/SimClock.hpp>
#include <World/WorldData.hpp>

namespace slr
{

	class HudBar
	{
	public:
		static constexpr float BarHeight = 40.0f;

		TimeChromeResult draw(const SimClock& clock, long long tick, const WorldData& world) const;

	private:
		TimeChrome _timeChrome;
		SituationChrome _situationChrome;
	};

}
