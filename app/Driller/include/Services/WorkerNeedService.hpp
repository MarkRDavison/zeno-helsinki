#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <optional>

namespace drl
{

	class IWorkerNeedService : public IGameTickService
	{
	public:
		~IWorkerNeedService() override = 0;

		virtual NeedBand classify(const WorkerInstance& worker, NeedId needId) const = 0;
		virtual bool hasCollapsedNeed(const WorkerInstance& worker) const = 0;
		virtual std::optional<NeedId> chosenSeekNeed(const WorkerInstance& worker) const = 0;
	};

	inline IWorkerNeedService::~IWorkerNeedService() = default;

	class WorkerNeedService : public IWorkerNeedService
	{
	public:
		WorkerNeedService(
			WorkerData& workerData,
			const INeedPrototypeService& needPrototypes,
			JobData& jobData,
			const IJobPrototypeService& jobPrototypes);
		~WorkerNeedService() override = default;

		void update(float delta) override;
		NeedBand classify(const WorkerInstance& worker, NeedId needId) const override;
		bool hasCollapsedNeed(const WorkerInstance& worker) const override;
		std::optional<NeedId> chosenSeekNeed(const WorkerInstance& worker) const override;

	private:
		NeedDecayModifier decayModifier(const WorkerInstance& worker, NeedId needId) const;
		const JobPrototype* workingJobPrototype(const WorkerInstance& worker) const;
		void applyRestore(WorkerInstance& worker, float delta);
		void leaveIfRestored(WorkerInstance& worker);
		void preemptIfNeeded(WorkerInstance& worker);
		void unassign(WorkerInstance& worker);
		bool currentJobRestores(const WorkerInstance& worker, NeedId needId) const;

		WorkerData& _workerData;
		const INeedPrototypeService& _needPrototypes;
		JobData& _jobData;
		const IJobPrototypeService& _jobPrototypes;
	};

}
