#pragma once

#include <Views/MainView.hpp>
#include <Views/SituationChrome.hpp>
#include <Views/TimeChrome.hpp>
#include <World/SimClock.hpp>
#include <World/WorldData.hpp>
#include <optional>

namespace slr
{

	struct HudBarResult
	{
		TimeChromeResult time;
		bool openPolicyEditor{ false };
		bool openDesignEditor{ false };
		bool openResearchEditor{ false };
		bool openSystemView{ false };
		bool returnToGalaxy{ false };
	};

	class HudBar
	{
	public:
		static constexpr float BarHeight = 40.0f;

		HudBarResult draw(
			const SimClock& clock,
			long long tick,
			const WorldData& world,
			MainViewKind mainViewKind,
			std::optional<long long> selectedSystemId) const;

	private:
		TimeChrome _timeChrome;
		SituationChrome _situationChrome;
	};

}
