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
			return "missile-standard";
		}

		if (goodId == MissileLongRangeGoodId)
		{
			return "missile-long-range";
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

		if (goodId == MissileEngineMk2GoodId)
		{
			return "missile engine mk2";
		}

		if (goodId == PrototypeMissileMk2GoodId)
		{
			return "prototype missile mk2";
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
		case MissileLongRangeGoodId:
			return TruckCapacity_Missiles;
		case MissileEngineMk1GoodId:
		case MissileWarheadMk1GoodId:
		case MissileSeekerMk1GoodId:
		case PrototypeMissileMk1GoodId:
		case MissileEngineMk2GoodId:
		case PrototypeMissileMk2GoodId:
			return TruckCapacity_Steel;
		default:
			return TruckCapacity_Steel;
		}
	}

	long long variantOfGoodId(long long goodId)
	{
		if (goodId == PrototypeMissileMk2GoodId)
		{
			return PrototypeMissileMk1GoodId;
		}

		if (goodId == MissileEngineMk2GoodId)
		{
			return MissileEngineMk1GoodId;
		}

		return 0;
	}

	bool goodsAreDeclaredVariants(long long a, long long b)
	{
		if (a == 0 || b == 0 || a == b)
		{
			return false;
		}

		return variantOfGoodId(a) == b || variantOfGoodId(b) == a;
	}

	bool isCombatMissileGood(long long goodId)
	{
		return goodId == MissileGoodId || goodId == MissileLongRangeGoodId;
	}

	float combatMissileRangeScale(long long goodId)
	{
		if (goodId == MissileLongRangeGoodId)
		{
			return CombatMissileRangeScale_LongRange;
		}

		if (goodId == MissileGoodId)
		{
			return CombatMissileRangeScale_Standard;
		}

		return 0.0f;
	}

	GoodAmount magazineUsed(const std::vector<GoodQuantity>& magazine)
	{
		GoodAmount used = 0.f;
		for (const auto& row : magazine)
		{
			used += row.amount;
		}

		return used;
	}

	GoodAmount magazineCount(const std::vector<GoodQuantity>& magazine, long long goodId)
	{
		GoodAmount amount = 0.f;
		for (const auto& row : magazine)
		{
			if (row.goodId == goodId)
			{
				amount += row.amount;
			}
		}

		return amount;
	}

	void addMagazineGood(std::vector<GoodQuantity>& magazine, long long goodId, GoodAmount amount)
	{
		if (amount <= 0.f)
		{
			return;
		}

		for (auto& row : magazine)
		{
			if (row.goodId == goodId)
			{
				row.amount += amount;
				return;
			}
		}

		magazine.push_back(GoodQuantity{ .goodId = goodId, .amount = amount });
	}

	bool spendMagazineGood(std::vector<GoodQuantity>& magazine, long long goodId, GoodAmount amount)
	{
		if (amount <= 0.f)
		{
			return false;
		}

		for (auto row = magazine.begin(); row != magazine.end(); ++row)
		{
			if (row->goodId != goodId)
			{
				continue;
			}

			if (row->amount < amount)
			{
				return false;
			}

			row->amount -= amount;
			if (row->amount <= 0.f)
			{
				magazine.erase(row);
			}

			return true;
		}

		return false;
	}

	long long highestRangeLoadedCombatMissileGoodId(const std::vector<GoodQuantity>& magazine)
	{
		long long bestGoodId = 0;
		float bestScale = -1.0f;
		for (const auto& row : magazine)
		{
			if (row.amount < 1.f || !isCombatMissileGood(row.goodId))
			{
				continue;
			}

			const float scale = combatMissileRangeScale(row.goodId);
			if (scale > bestScale)
			{
				bestScale = scale;
				bestGoodId = row.goodId;
			}
		}

		return bestGoodId;
	}

}
