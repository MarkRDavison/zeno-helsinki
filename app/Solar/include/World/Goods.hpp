#pragma once

namespace slr
{

	using GoodAmount = float;

	constexpr long long SteelGoodId = 1;
	constexpr long long IronOreGoodId = 2;
	constexpr long long MissileGoodId = 3;
	constexpr long long MissileEngineMk1GoodId = 4;
	constexpr long long MissileWarheadMk1GoodId = 5;
	constexpr long long MissileSeekerMk1GoodId = 6;
	constexpr long long PrototypeMissileMk1GoodId = 7;

	constexpr long long HelionSystemId = 1;
	constexpr long long KestrelSystemId = 2;
	constexpr long long JunctionSystemId = 3;
	constexpr long long AvonSystemId = 9;
	constexpr GoodAmount AvonStarterSteelStockpile = 10000.f;
	constexpr GoodAmount AvonComponentMaxStockpile = 200.f;
	constexpr GoodAmount AvonPrototypeMissileMaxStockpile = 200.f;
	constexpr GoodAmount FactoryAvonEngineSteelPerTick = 2.f;
	constexpr GoodAmount FactoryAvonWarheadSteelPerTick = 3.f;
	constexpr GoodAmount FactoryAvonSeekerSteelPerTick = 2.f;
	constexpr GoodAmount FactoryAvonComponentOutputPerTick = 1.f;
	constexpr GoodAmount FactoryAvonAssemblerComponentInputPerTick = 1.f;
	constexpr GoodAmount FactoryAvonAssemblerOutputPerTick = 1.f;
	constexpr long long FactoryAvonAssemblerTicksPerOutput = 2;

	constexpr long long FactoryRecipeId_MissileEngineMk1 = 101;
	constexpr long long FactoryRecipeId_MissileWarheadMk1 = 102;
	constexpr long long FactoryRecipeId_MissileSeekerMk1 = 103;
	constexpr long long FactoryRecipeId_PrototypeMissileMk1 = 104;
	constexpr GoodAmount TruckCapacity_Steel = 50.f;
	constexpr GoodAmount TruckCapacity_IronOre = 50.f;
	constexpr GoodAmount TruckCapacity_Missiles = 50.f;
	constexpr GoodAmount MineMaxStockpile_Steel = 4000.f;
	constexpr GoodAmount MineMaxStockpile_IronOre = 4000.f;
	constexpr GoodAmount FactoryMaxStockpile_Steel = 4000.f;
	constexpr GoodAmount FactoryMaxStockpile_Missiles = 4000.f;
	constexpr GoodAmount FactoryInputOrePerTick = 50.f;
	constexpr GoodAmount FactoryOutputSteelPerTick = 50.f;
	constexpr GoodAmount FactoryMissileInputSteelPerTick = 10.f;
	constexpr GoodAmount FactoryMissileOutputPerTick = 10.f;
	constexpr long long FactoryFullEfficiencyPerThousand = 1000;
	constexpr long long FactoryStartEfficiencyPerThousand = 100;
	constexpr GoodAmount FactoryRampUnitsPerEfficiencyStep = 10.f;
	constexpr long long FactoryRampEfficiencyStepPerThousand = 10;
	constexpr GoodAmount FactoryMissileSteelDesiredPile = 500.f;
	constexpr GoodAmount YardPopulationConsumePerTick_Steel = 10.f;
	constexpr GoodAmount YardPopulationReserved_Steel = 200.f;
	constexpr GoodAmount YardSlipSteelCost = 200.f; // StartYardSlip spends this immediately from available steel
	constexpr long long YardSlipBuildTicks = 8; // remainingBuildTicks on a new slip; 0 is idle
	constexpr GoodAmount TruckHullSteelCost = 1000.f;
	constexpr long long TruckHullBuildTicks = 10; // per-tick steel is TruckHullSteelCost / TruckHullBuildTicks
	constexpr GoodAmount RefitHullSteelCost = 1000.f;
	constexpr long long RefitHullBuildTicks = 50; // per-tick steel is RefitHullSteelCost / RefitHullBuildTicks
	constexpr long long CivilianHazardFadeTicks = 8;
	constexpr GoodAmount YardStarterMissileStockpile = 2000.f;

	const char* goodName(long long goodId);
	GoodAmount truckloadForGood(long long goodId);

}
