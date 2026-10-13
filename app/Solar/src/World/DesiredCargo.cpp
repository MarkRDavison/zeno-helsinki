#include <World/DesiredCargo.hpp>
#include <algorithm>

namespace slr
{

	DesiredCargo desiredCargoFillRemaining(long long goodId)
	{
		DesiredCargo desired;
		desired.rows.push_back(DesiredCargoRow{
			.goodId = goodId,
			.quota = 0.f,
			.fill = DesiredCargoFill::FillRemaining });
		return desired;
	}

	bool desiredCargoIsValidForMunitionsSlice(const DesiredCargo& desired)
	{
		long long fillRemainingRows = 0;
		for (const auto& row : desired.rows)
		{
			if (!isCombatMissileGood(row.goodId) || row.quota < 0.f)
			{
				return false;
			}

			if (row.fill == DesiredCargoFill::FillRemaining)
			{
				++fillRemainingRows;
			}
		}

		return fillRemainingRows <= 1;
	}

	GoodAmount desiredCargoReservedCapacity(const DesiredCargo& desired, GoodAmount capacity)
	{
		GoodAmount reserved = 0.f;
		for (const auto& row : desired.rows)
		{
			if (row.fill == DesiredCargoFill::Quota)
			{
				reserved += row.quota;
			}
		}

		return std::min(reserved, capacity);
	}

	long long nextDesiredCargoGoodId(
		const DesiredCargo& desired,
		const std::vector<GoodQuantity>& currentCounts,
		GoodAmount capacity,
		const std::vector<GoodQuantity>& available)
	{
		if (!desiredCargoIsValidForMunitionsSlice(desired) || capacity < 1.f)
		{
			return 0;
		}

		const GoodAmount reserved = desiredCargoReservedCapacity(desired, capacity);
		for (const auto& row : desired.rows)
		{
			if (row.fill != DesiredCargoFill::Quota)
			{
				continue;
			}

			if (magazineCount(currentCounts, row.goodId) + 1.f > row.quota)
			{
				continue;
			}

			if (magazineCount(available, row.goodId) < 1.f)
			{
				continue;
			}

			return row.goodId;
		}

		const GoodAmount fillerCap = capacity - reserved;
		for (const auto& row : desired.rows)
		{
			if (row.fill != DesiredCargoFill::FillRemaining)
			{
				continue;
			}

			if (fillerCap < 1.f || magazineCount(currentCounts, row.goodId) + 1.f > fillerCap)
			{
				continue;
			}

			if (magazineCount(available, row.goodId) < 1.f)
			{
				continue;
			}

			return row.goodId;
		}

		return 0;
	}

}
