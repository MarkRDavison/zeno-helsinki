#pragma once

namespace slr
{

	constexpr long long SteelGoodId = 1;
	constexpr long long IronOreGoodId = 2;
	constexpr long long MissileGoodId = 3;
	constexpr long long TruckCapacity_Steel = 50;
	constexpr long long TruckCapacity_IronOre = 50;
	constexpr long long TruckCapacity_Missiles = 50;
	constexpr long long MineMaxStockpile_Steel = 4000;
	constexpr long long MineMaxStockpile_IronOre = 4000;
	constexpr long long FactoryMaxStockpile_Steel = 4000;
	constexpr long long FactoryInputOrePerTick = 50;
	constexpr long long FactoryOutputSteelPerTick = 50;
	constexpr long long YardPopulationConsumePerTick_Steel = 10;
	constexpr long long YardPopulationReserved_Steel = 200;
	constexpr long long YardSlipSteelCost = 200; // StartYardSlip spends this immediately from available steel
	constexpr long long YardSlipBuildTicks = 8; // remainingBuildTicks on a new slip; 0 is idle
	constexpr long long TruckHullSteelCost = 1000;
	constexpr long long TruckHullBuildTicks = 10; // per-tick steel is TruckHullSteelCost / TruckHullBuildTicks
	constexpr long long RefitHullSteelCost = 1000;
	constexpr long long RefitHullBuildTicks = 50; // per-tick steel is RefitHullSteelCost / RefitHullBuildTicks
	constexpr long long CivilianHazardFadeTicks = 8;
	constexpr long long YardStarterMissileStockpile = 2000;

	const char* goodName(long long goodId);
	long long truckloadForGood(long long goodId);

}
