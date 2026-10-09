#pragma once

#include <helsinki/System/glm.hpp>

namespace drl
{

	struct DirtFillPushConstantObject
	{
		glm::vec2 origin;
		float tileSize;
		int cameraIndex;
		glm::vec2 aabbMin;
		glm::vec2 aabbMax;
	};

}
