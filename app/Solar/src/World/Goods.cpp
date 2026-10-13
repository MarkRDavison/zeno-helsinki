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

		if (goodId == MissileEngineMk1GoodId)
		{
			return "missile engine mk1";
		}

		if (goodId == MissileWarheadMk1GoodId)
		{
			return "missile warhead mk1";
		}

		if (goodId == MissileSeekerMk1GoodId)
		{
			return "missile seeker mk1";
		}

		if (goodId == PrototypeMissileMk1GoodId)
		{
			return "prototype missile mk1";
		}

		return "unknown";
	}

	GoodAmount truckloadForGood(long long goodId)
	{
		switch (goodId)
		{
		case IronOreGoodId:
			return TruckCapacity_IronOre;
		case SteelGoodId:
			return TruckCapacity_Steel;
		case MissileGoodId:
			return TruckCapacity_Missiles;
		case MissileEngineMk1GoodId:
		case MissileWarheadMk1GoodId:
		case MissileSeekerMk1GoodId:
		case PrototypeMissileMk1GoodId:
			return TruckCapacity_Steel;
		default:
			return TruckCapacity_Steel;
		}
	}

}
