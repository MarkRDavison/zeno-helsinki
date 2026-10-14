#pragma once

#include <World/WorldData.hpp>

namespace slr
{

	constexpr float CombatEngagementLength = 100.0f;
	constexpr int CombatMaxInnerTicks = 10000;

	void applyLaserDamage(long long& shields, long long& hull, long long damage);
	void applyEncounterStance(WorldData& world);
	void resolveBattles(WorldData& world);

}
