#pragma once

#include <helsinki/System/glm.hpp>

namespace drl
{

	inline glm::vec4 placementGhostColor(bool canPlace, bool canAfford)
	{
		if (canPlace && canAfford)
		{
			return glm::vec4(0.35f, 1.0f, 0.35f, 0.5f);
		}

		return glm::vec4(1.0f, 0.35f, 0.35f, 0.5f);
	}

}
