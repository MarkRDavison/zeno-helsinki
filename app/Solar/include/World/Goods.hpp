#pragma once

namespace slr
{

	constexpr long long SteelGoodId = 1;
	constexpr long long IronOreGoodId = 2;
	constexpr long long TruckCapacity_Steel = 50;
	constexpr long long TruckCapacity_IronOre = 50;
	constexpr long long MineMaxStockpile_Steel = 4000;
	constexpr long long MineMaxStockpile_IronOre = 4000;
	constexpr long long FactoryMaxStockpile_Steel = 4000;
	constexpr long long FactoryInputOrePerTick = 50;
	constexpr long long FactoryOutputSteelPerTick = 50;

	const char* goodName(long long goodId);
	long long truckloadForGood(long long goodId);

}
