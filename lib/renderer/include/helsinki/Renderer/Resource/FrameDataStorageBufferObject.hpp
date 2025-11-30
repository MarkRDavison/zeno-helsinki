#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{
	struct FrameDataStorageBufferObject
	{
		alignas(16) glm::vec4 uvRect;
	};
}