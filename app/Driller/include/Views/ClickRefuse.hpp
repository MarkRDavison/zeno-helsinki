#pragma once

#include <Views/ShaftInspect.hpp>
#include <helsinki/System/glm.hpp>
#include <format>
#include <string>
#include <string_view>

namespace drl
{

	inline constexpr const char* kClickRefusePlaceInvalid = "Can't place here";

	enum class ClickRefuseKind
	{
		None,
		ShaftBroke,
		PlaceBroke,
		PlaceInvalid
	};

	struct ClickRefuse
	{
		ClickRefuseKind kind{ ClickRefuseKind::None };
		std::string message;
		glm::ivec2 tile{ 0, 0 };
	};

	inline std::string formatCantAffordPlace(long long cost, std::string_view moneyLabel)
	{
		return std::format("Can't afford {} {}", cost, moneyLabel);
	}

	inline ClickRefuse classifyShaftClick(
		glm::ivec2 tile,
		int shaftLevel,
		bool canAfford)
	{
		ClickRefuse refuse{};
		refuse.tile = tile;
		if (tile.y != nextShaftLevel(shaftLevel) || canAfford)
		{
			return refuse;
		}

		refuse.kind = ClickRefuseKind::ShaftBroke;
		return refuse;
	}

	inline ClickRefuse classifyPlaceRefuse(
		glm::ivec2 tile,
		bool canAfford,
		bool executeOk)
	{
		ClickRefuse refuse{};
		refuse.tile = tile;
		if (executeOk)
		{
			return refuse;
		}

		if (!canAfford)
		{
			refuse.kind = ClickRefuseKind::PlaceBroke;
			return refuse;
		}

		refuse.kind = ClickRefuseKind::PlaceInvalid;
		refuse.message = kClickRefusePlaceInvalid;
		return refuse;
	}

}
