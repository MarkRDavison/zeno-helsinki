#pragma once

#include <Entities/Building.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <vector>

namespace drl
{

	struct BuildingData
	{
		std::vector<BuildingInstance> buildings;
	};

	inline long long workerHousingCapacity(
		const BuildingData& buildingData,
		const IBuildingPrototypeService& prototypes)
	{
		long long capacity = 0;
		for (const BuildingInstance& building : buildingData.buildings)
		{
			const auto beds = buildingMetadataInt(
				prototypes.getPrototype(building.prototypeId),
				kBuildingMetadataWorkerCapacity);
			if (beds)
			{
				capacity += *beds;
			}
		}

		return capacity;
	}

}
