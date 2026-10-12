#pragma once

namespace slr
{

	constexpr long long SteelGoodId = 1;
	constexpr long long TruckCapacity_Steel = 50;
	constexpr long long MineMaxStockpile_Steel = 4000;

	const char* goodName(long long goodId);
	long long truckloadForGood(long long goodId);

}
