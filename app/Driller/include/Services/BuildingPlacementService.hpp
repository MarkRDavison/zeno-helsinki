#pragma once

#include <Entities/Building.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{

	class IBuildingPlacementService
	{
	public:
		virtual ~IBuildingPlacementService() = 0;

		virtual bool canPlacePrototype(BuildingPrototypeId prototypeId, int level, int column) const = 0;
		virtual bool placePrototype(BuildingPrototypeId prototypeId, int level, int column) = 0;
	};

	inline IBuildingPlacementService::~IBuildingPlacementService() = default;

	class BuildingPlacementService : public IBuildingPlacementService
	{
	public:
		BuildingPlacementService(
			BuildingData& buildingData,
			ITerrainAlterationService& terrain,
			IWorkerRecruitmentService& recruitment,
			IJobCreationService& jobs,
			IBuildingPrototypeService& buildingPrototypes);
		~BuildingPlacementService() override = default;

		bool canPlacePrototype(BuildingPrototypeId prototypeId, int level, int column) const override;
		bool placePrototype(BuildingPrototypeId prototypeId, int level, int column) override;

	private:
		BuildingData& _buildingData;
		ITerrainAlterationService& _terrain;
		IWorkerRecruitmentService& _recruitment;
		IJobCreationService& _jobs;
		IBuildingPrototypeService& _buildingPrototypes;
	};

}
