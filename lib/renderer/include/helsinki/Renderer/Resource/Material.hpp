#pragma once

#include <helsinki/System/glm.hpp>
#include <string>

namespace hl
{

	struct Material
	{
		std::string name;
		glm::vec3 diffuse{ 1.0f, 1.0f, 1.0f };
		glm::vec3 specular{ 0.0f, 0.0f, 0.0f };
		float shininess{ 0.0f };
		std::string diffuseTex;
	};

}