#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>

namespace drl
{

	class IWorkerJobUpdateService : public IGameTickService
	{
	public:
		~IWorkerJobUpdateService() override = 0;
	};

	inline IWorkerJobUpdateService::~IWorkerJobUpdateService() = default;

	class WorkerJobUpdateService : public IWorkerJobUpdateService
	{
	public:
		WorkerJobUpdateService(
			WorkerData& workerData,
			JobData& jobData,
			ITerrainAlterationService& terrain,
			const IJobPrototypeService& jobPrototypes);
		~WorkerJobUpdateService() override = default;

		void update(float delta) override;
		void updateWorkerJob(float delta, WorkerInstance& worker, JobInstance& job);
		void removeCompletedJobs();

	private:
		WorkerData& _workerData;
		JobData& _jobData;
		ITerrainAlterationService& _terrain;
		const IJobPrototypeService& _jobPrototypes;
	};

}
