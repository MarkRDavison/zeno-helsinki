#pragma once

#include <World/WorldData.hpp>

namespace slr
{

	constexpr long long StarterCivilianTruckCount = 32;
	constexpr long long StarterMilitaryTruckCount = 8;
	constexpr long long HelionCivilianTruckClassId = 5;
	constexpr long long HelionMilitaryTruckClassId = 6;
	constexpr long long AshreachCivilianTruckClassId = 7;
	constexpr long long AshreachMilitaryTruckClassId = 8;

	WorldData makeStarterMap();

}
