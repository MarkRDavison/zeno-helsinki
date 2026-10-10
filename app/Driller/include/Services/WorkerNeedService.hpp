#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>

namespace drl
{

	class IWorkerNeedService : public IGameTickService
	{
	public:
		~IWorkerNeedService() override = 0;
	};

	inline IWorkerNeedService::~IWorkerNeedService() = default;

	class WorkerNeedService : public IWorkerNeedService
	{
	public:
		WorkerNeedService(
			WorkerData& workerData,
			const INeedPrototypeService& needPrototypes,
			const JobData& jobData,
			const IJobPrototypeService& jobPrototypes);
		~WorkerNeedService() override = default;

		void update(float delta) override;

	private:
		NeedDecayModifier decayModifier(const WorkerInstance& worker, NeedId needId) const;

		WorkerData& _workerData;
		const INeedPrototypeService& _needPrototypes;
		const JobData& _jobData;
		const IJobPrototypeService& _jobPrototypes;
	};

}
