#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{

	struct alignas(16) TextPushConstantObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec4 colour;
		alignas(16) uint32_t fontAtlasIndex;
		alignas(4) int cameraIndex{ 0 };
		uint32_t pad[2];
	};

}