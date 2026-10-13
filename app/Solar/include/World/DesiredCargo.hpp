#pragma once

#include <World/Goods.hpp>
#include <vector>

namespace slr
{

	enum class DesiredCargoFill
	{
		Quota,
		FillRemaining
	};

	enum class DesiredCargoTargetType
	{
		FleetMagazine,
		StandingConvoyHold
	};

	struct DesiredCargoRow
	{
		long long goodId{ 0 };
		GoodAmount quota{ 0.f };
		DesiredCargoFill fill{ DesiredCargoFill::Quota };
	};

	struct DesiredCargo
	{
		std::vector<DesiredCargoRow> rows;
	};

	DesiredCargo desiredCargoFillRemaining(long long goodId);
	bool desiredCargoIsValidForMunitionsSlice(const DesiredCargo& desired);
	GoodAmount desiredCargoReservedCapacity(const DesiredCargo& desired, GoodAmount capacity);
	long long nextDesiredCargoGoodId(
		const DesiredCargo& desired,
		const std::vector<GoodQuantity>& currentCounts,
		GoodAmount capacity,
		const std::vector<GoodQuantity>& available);

}
