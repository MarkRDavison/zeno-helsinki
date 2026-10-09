#pragma once

#include <helsinki/System/glm.hpp>

#include <optional>

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

		glm::vec3 foreground{ 0.95f, 0.95f, 0.97f };
		glm::vec3 muted{ 0.75f, 0.76f, 0.80f };
		glm::vec3 background{ 0.12f, 0.13f, 0.16f };
		glm::vec3 surface{ 0.18f, 0.19f, 0.24f };
		glm::vec3 well{ 0.25f, 0.28f, 0.32f };
		glm::vec3 hover{ 0.32f, 0.34f, 0.42f };
		glm::vec3 accent{ 1.0f, 0.5f, 0.0f };
		glm::vec3 border{ 0.08f, 0.09f, 0.12f };
		glm::vec4 scrim{ 0.0f, 0.0f, 0.0f, 0.55f };

		float padding = 8.0f;
		float paddingLarge = 16.0f;
		float gap = 8.0f;
		float borderWidth = 0.0f;

		float controlWidth = 280.0f;
		float controlHeight = 32.0f;
		glm::vec2 viewportSize{ 280.0f, 240.0f };
	};

	Theme& theme();

	template<typename T>
	T resolve(const std::optional<T>& value, T fallback)
	{
		return value.value_or(fallback);
	}
}
