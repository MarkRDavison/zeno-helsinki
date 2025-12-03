#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{

	struct alignas(16) TextPushConstantObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) uint32_t fontAtlasIndex;
		uint32_t pad[3]; // fills remaining bytes to 16
	};

}