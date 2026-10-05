#pragma once

#include <helsinki/System/glm.hpp>

namespace sk
{
	inline constexpr const char* RotateTag = "ROTATE";

	struct Prop
	{
		const char* modelId;
		glm::vec3 position;
		bool rotate;
	};

	inline constexpr Prop SceneProps[] = {
		{ "plane", { 0.0f, 0.0f, 0.0f }, false },
		{ "rock_crystals", { -1.0f, 0.0f, -1.0f }, true },
		{ "satelliteDish_detailed", { -1.0f, 0.0f, 1.0f }, true },
		{ "turret_double", { 1.0f, 0.0f, 1.0f }, true },
	};
}
