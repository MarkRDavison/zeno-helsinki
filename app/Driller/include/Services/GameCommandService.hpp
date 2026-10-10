#pragma once

#include <Core/GameCommand.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/UpgradeService.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/WorkerCreationService.hpp>

namespace drl
{

	inline constexpr long long kPlayerShaftDigCostPerLevel = 100;
	inline constexpr const char* kJobBuildBuilding = "Job_Build_Building";

	class IGameCommandService
	{
	public:
		virtual ~IGameCommandService() = 0;

		virtual bool execute(const GameCommand& command) = 0;
		virtual void tick() = 0;
		virtual long long currentTick() const = 0;
	};

	inline IGameCommandService::~IGameCommandService() = default;

	class GameCommandService : public IGameCommandService
	{
	public:
		GameCommandService(
			ITerrainAlterationService& terrain,
			IEconomyResourceService& economy,
			IJobCreationService& jobs,
			IWorkerCreationService& workers,
			IBuildingPlacementService& buildings,
			IBuildingPrototypeService& buildingPrototypes,
			IShuttleCreationService& shuttles,
			IUpgradeService& upgrades,
			WorkerData& workerData);
		~GameCommandService() override = default;

		bool execute(const GameCommand& command) override;
		void tick() override;
		long long currentTick() const override;

	private:
		bool handleDigShaft(CommandSource source, const DigShaft& event);
		bool handleDigTile(const DigTile& event);
		bool handleAddResource(const AddResource& event);
		bool handleCreateJob(CommandSource source, const CreateJob& event);
		bool handleCreateWorker(const CreateWorker& event);
		bool handlePlaceBuilding(const PlaceBuilding& event);
		bool handleCreateShuttle(const CreateShuttle& event);
		bool handleAddUpgrade(const AddUpgrade& event);
		bool handleCancelJob(CommandSource source, const CancelJob& event);

		ITerrainAlterationService& _terrain;
		IEconomyResourceService& _economy;
		IJobCreationService& _jobs;
		IWorkerCreationService& _workers;
		IBuildingPlacementService& _buildings;
		IBuildingPrototypeService& _buildingPrototypes;
		IShuttleCreationService& _shuttles;
		IUpgradeService& _upgrades;
		WorkerData& _workerData;
		long long _tick{ 0 };
	};

}
