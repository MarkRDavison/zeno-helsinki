#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/TerrainAlterationService.hpp>

namespace drl
{

	class IWorkerMovementService : public IGameTickService
	{
	public:
		~IWorkerMovementService() override = 0;
	};

	inline IWorkerMovementService::~IWorkerMovementService() = default;

	class WorkerMovementService : public IWorkerMovementService
	{
	public:
		WorkerMovementService(
			WorkerData& workerData,
			const JobData& jobData,
			const ITerrainAlterationService& terrain,
			const ShuttleData& shuttleData);
		~WorkerMovementService() override = default;

		void update(float delta) override;
		void updateWorker(float delta, WorkerInstance& worker);

		void updateIdleWorker(float delta, WorkerInstance& worker);
		void updateMovingToJobWorker(float delta, WorkerInstance& worker);
		void updateWanderingWorker(float delta, WorkerInstance& worker);
		void updateLeavingWorker(float delta, WorkerInstance& worker);

	private:
		WorkerData& _workerData;
		const JobData& _jobData;
		const ITerrainAlterationService& _terrain;
		const ShuttleData& _shuttleData;
	};

}
