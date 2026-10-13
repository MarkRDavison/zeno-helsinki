#include <World/Goods.hpp>

namespace slr
{

	const char* goodName(long long goodId)
	{
		if (goodId == SteelGoodId)
		{
			return "steel";
		}

		if (goodId == IronOreGoodId)
		{
			return "iron ore";
		}

		if (goodId == MissileGoodId)
		{
			return "missiles";
		}

		return "unknown";
	}

	long long truckloadForGood(long long goodId)
	{
		switch (goodId)
		{
		case IronOreGoodId:
			return TruckCapacity_IronOre;
		case SteelGoodId:
			return TruckCapacity_Steel;
		case MissileGoodId:
			return TruckCapacity_Missiles;
		default:
			return TruckCapacity_Steel;
		}
	}

}
