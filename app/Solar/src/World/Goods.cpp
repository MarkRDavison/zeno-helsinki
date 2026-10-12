#include <World/Goods.hpp>

namespace slr
{

	const char* goodName(long long goodId)
	{
		if (goodId == SteelGoodId)
		{
			return "steel";
		}

		return "unknown";
	}

	long long truckloadForGood(long long goodId)
	{
		switch (goodId)
		{
		case SteelGoodId:
			return TruckCapacity_Steel;
		default:
			return TruckCapacity_Steel;
		}
	}

}
