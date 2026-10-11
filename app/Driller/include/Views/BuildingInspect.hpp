#pragma once

#include <Entities/Building.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace drl
{

	struct BuildingSpriteBox
	{
		float left{ 0.0f };
		float top{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
	};

	struct BuildingInspectJob
	{
		std::string label;
		int inUse{ 0 };
		int total{ 0 };
	};

	struct BuildingInspectSnapshot
	{
		std::string label;
		std::optional<long long> beds;
		std::vector<BuildingInspectJob> jobs;
	};

	BuildingSpriteBox buildingSpriteBox(
		const BuildingInstance& building,
		const BuildingPrototype& prototype,
		float originX,
		float originY,
		float tileSize);

	bool buildingContainsTile(
		const BuildingInstance& building,
		const BuildingPrototype& prototype,
		glm::ivec2 tile);

	std::optional<BuildingId> pickPlacedBuildingAt(
		const BuildingData& buildings,
		const IBuildingPrototypeService& buildingPrototypes,
		glm::ivec2 tile);

	std::optional<BuildingInspectSnapshot> makeBuildingInspectSnapshot(
		BuildingId buildingId,
		const BuildingData& buildings,
		const IBuildingPrototypeService& buildingPrototypes,
		const JobData& jobs,
		const IJobPrototypeService& jobPrototypes);

}
