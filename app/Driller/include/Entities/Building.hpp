#pragma once

#include <helsinki/System/glm.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace drl
{

	inline constexpr const char* kBuildingMetadataWorkerCapacity = "workerCapacity";

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
		std::string label;
		long long cost{ 0 };
		glm::ivec2 size{ 0, 0 };
		glm::ivec2 texture{ 0, 0 };
		std::unordered_map<std::string, int> requiredWorkers;
		std::vector<std::pair<std::string, glm::vec2>> providedJobs;
		std::unordered_map<std::string, long long> metadata;
	};

	inline std::optional<long long> buildingMetadataInt(
		const BuildingPrototype& prototype,
		std::string_view key)
	{
		const auto it = prototype.metadata.find(std::string(key));
		if (it == prototype.metadata.end())
		{
			return std::nullopt;
		}

		return it->second;
	}

}
