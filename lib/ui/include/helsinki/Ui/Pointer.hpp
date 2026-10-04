#pragma once

#include <helsinki/System/glm.hpp>

namespace hl::ui
{
	enum class EventResult
	{
		Ignore,
		Consume
	};

	struct Pointer
	{
		glm::vec2 position{ 0.0f };
		bool primaryDown = false;
		bool primaryReleased = false;
	};
}
