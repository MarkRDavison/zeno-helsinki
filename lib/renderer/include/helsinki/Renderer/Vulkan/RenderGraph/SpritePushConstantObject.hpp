#pragma once

#include <helsinki/System/glm.hpp>

namespace hl
{

	struct alignas(16) SpritePushConstantObject
	{
		alignas(16) glm::mat4 model;
		alignas(8) glm::vec2 size;
		alignas(8) glm::vec2 offset;
		alignas(4) int frameIndex;
		alignas(4) int cameraIndex;
	};

}