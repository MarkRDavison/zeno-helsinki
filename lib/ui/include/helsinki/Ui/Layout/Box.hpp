#pragma once

#include <helsinki/System/glm.hpp>

namespace hl::ui
{
	struct Box
	{
		glm::vec2 pos{ 0.0f };
		glm::vec2 size{ 0.0f };

		Box() = default;
		constexpr Box(glm::vec2 pos, glm::vec2 size) : pos(pos), size(size) {}
		constexpr Box(float x, float y, float width, float height) : pos(x, y), size(width, height) {}

		bool contains(glm::vec2 point) const
		{
			return point.x >= pos.x && point.x <= pos.x + size.x
				&& point.y >= pos.y && point.y <= pos.y + size.y;
		}
	};

	struct Edges
	{
		float left = 0.0f;
		float top = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;

		static Edges all(float value)
		{
			return { value, value, value, value };
		}
	};

	enum class Kind
	{
		Absolute,
		Column,
		Row
	};

	enum class Align
	{
		Start,
		Center,
		End
	};
}
