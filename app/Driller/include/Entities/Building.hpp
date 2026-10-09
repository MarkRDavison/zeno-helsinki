#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace drl
{

	using BuildingId = long long;
	using BuildingPrototypeId = long long;

	struct BuildingInstance
	{
		BuildingId id{ 0 };
		glm::ivec2 coordinates{ 0, 0 };
		BuildingPrototypeId prototypeId{ 0 };
	};

	struct BuildingPrototype
	{
		std::string name;
		glm::ivec2 size{ 0, 0 };
		glm::ivec2 texture{ 0, 0 };
		std::unordered_map<std::string, int> requiredWorkers;
		std::vector<std::pair<std::string, glm::vec2>> providedJobs;
	};

}
