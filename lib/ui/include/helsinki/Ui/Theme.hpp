#pragma once

namespace hl::ui
{
	enum class TooltipPlacement
	{
		Below,
		Above,
		Right,
		Left
	};

	struct Theme
	{
		unsigned fontSize = 16;
		float tooltipDelay = 0.4f;
		float tooltipOffset = 8.0f;
		TooltipPlacement tooltipPlacement = TooltipPlacement::Below;
	};

	Theme& theme();
}
