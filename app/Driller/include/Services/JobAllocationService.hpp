#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerPrototypeService.hpp>

namespace drl
{

	class IJobAllocationService : public IGameTickService
	{
	public:
		~IJobAllocationService() override = 0;

		virtual void allocateJobs(int number = -1) = 0;
	};

	inline IJobAllocationService::~IJobAllocationService() = default;

	class JobAllocationService : public IJobAllocationService
	{
	public:
		JobAllocationService(
			JobData& jobData,
			WorkerData& workerData,
			const ITerrainAlterationService& terrain,
			const IWorkerPrototypeService& workerPrototypes);
		~JobAllocationService() override = default;

		void update(float delta) override;
		void allocateJobs(int number = -1) override;

		bool canWorkerPerformJob(const WorkerInstance& worker, const JobInstance& job) const;

	private:
		JobData& _jobData;
		WorkerData& _workerData;
		const ITerrainAlterationService& _terrain;
		const IWorkerPrototypeService& _workerPrototypes;
	};

}
