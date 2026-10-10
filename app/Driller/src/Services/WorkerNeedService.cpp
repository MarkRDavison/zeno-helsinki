#include <Services/WorkerNeedService.hpp>
#include <algorithm>

namespace drl
{

	WorkerNeedService::WorkerNeedService(
		WorkerData& workerData,
		const INeedPrototypeService& needPrototypes,
		JobData& jobData,
		const IJobPrototypeService& jobPrototypes)
		: _workerData(workerData)
		, _needPrototypes(needPrototypes)
		, _jobData(jobData)
		, _jobPrototypes(jobPrototypes)
	{
	}

	NeedDecayModifier WorkerNeedService::decayModifier(const WorkerInstance& worker, NeedId needId) const
	{
		NeedDecayModifier modifier{};
		if (worker.state != WorkerState::WorkingJob || worker.allocatedJobId == 0)
		{
			return modifier;
		}

		const JobInstance& job = _jobData.getJob(worker.allocatedJobId);
		if (!_jobPrototypes.isPrototypeRegistered(job.prototypeId))
		{
			return modifier;
		}

		const JobPrototype& jobPrototype = _jobPrototypes.getPrototype(job.prototypeId);
		const auto it = jobPrototype.needDecay.find(needId);
		if (it == jobPrototype.needDecay.end())
		{
			return modifier;
		}

		return it->second;
	}

	NeedBand WorkerNeedService::classify(const WorkerInstance& worker, NeedId needId) const
	{
		const NeedPrototype& prototype = _needPrototypes.getPrototype(needId);
		const auto it = worker.needValues.find(needId);
		if (it == worker.needValues.end())
		{
			return NeedBand::Ok;
		}

		return classifyNeedValue(it->second, prototype);
	}

	bool WorkerNeedService::hasCollapsedNeed(const WorkerInstance& worker) const
	{
		for (const NeedId needId : _needPrototypes.registeredIds())
		{
			if (classify(worker, needId) == NeedBand::Collapse)
			{
				return true;
			}
		}

		return false;
	}

	std::optional<NeedId> WorkerNeedService::chosenSeekNeed(const WorkerInstance& worker) const
	{
		if (hasCollapsedNeed(worker))
		{
			return std::nullopt;
		}

		std::optional<NeedId> chosen;
		int bestPriority = 0;
		float bestValue = 0.0f;
		for (const NeedId needId : _needPrototypes.registeredIds())
		{
			if (classify(worker, needId) != NeedBand::Seek)
			{
				continue;
			}

			const NeedPrototype& prototype = _needPrototypes.getPrototype(needId);
			const float value = worker.needValues.at(needId);
			if (!chosen || prototype.priority < bestPriority
				|| (prototype.priority == bestPriority && value < bestValue))
			{
				chosen = needId;
				bestPriority = prototype.priority;
				bestValue = value;
			}
		}

		return chosen;
	}

	bool WorkerNeedService::hasFreeRestoreJob(NeedId needId) const
	{
		for (const JobInstance& job : _jobData.jobs)
		{
			if (job.allocatedWorkerId != 0)
			{
				continue;
			}

			if (!_jobPrototypes.isPrototypeRegistered(job.prototypeId))
			{
				continue;
			}

			if (_jobPrototypes.getPrototype(job.prototypeId).needRestore.contains(needId))
			{
				return true;
			}
		}

		return false;
	}

	bool WorkerNeedService::hasBlockedSeek() const
	{
		for (const WorkerInstance& worker : _workerData.workers)
		{
			const std::optional<NeedId> chosen = chosenSeekNeed(worker);
			if (!chosen)
			{
				continue;
			}

			if (currentJobRestores(worker, *chosen))
			{
				continue;
			}

			if (!hasFreeRestoreJob(*chosen))
			{
				return true;
			}
		}

		return false;
	}

	const JobPrototype* WorkerNeedService::workingJobPrototype(const WorkerInstance& worker) const
	{
		if (worker.state != WorkerState::WorkingJob || worker.allocatedJobId == 0)
		{
			return nullptr;
		}

		const JobInstance& job = _jobData.getJob(worker.allocatedJobId);
		if (!_jobPrototypes.isPrototypeRegistered(job.prototypeId))
		{
			return nullptr;
		}

		return &_jobPrototypes.getPrototype(job.prototypeId);
	}

	void WorkerNeedService::applyRestore(WorkerInstance& worker, float delta)
	{
		const JobPrototype* jobPrototype = workingJobPrototype(worker);
		if (jobPrototype == nullptr)
		{
			return;
		}

		for (const auto& [needId, restorePerSecond] : jobPrototype->needRestore)
		{
			const auto it = worker.needValues.find(needId);
			if (it == worker.needValues.end())
			{
				continue;
			}

			it->second += restorePerSecond * delta;
			it->second = std::clamp(it->second, 0.0f, kNeedValueFull);
		}
	}

	void WorkerNeedService::leaveIfRestored(WorkerInstance& worker)
	{
		const JobPrototype* jobPrototype = workingJobPrototype(worker);
		if (jobPrototype == nullptr || jobPrototype->needRestore.empty())
		{
			return;
		}

		for (const auto& [needId, restorePerSecond] : jobPrototype->needRestore)
		{
			const auto it = worker.needValues.find(needId);
			if (it == worker.needValues.end() || it->second < jobPrototype->restoreUntil)
			{
				return;
			}
		}

		unassign(worker);
	}

	bool WorkerNeedService::currentJobRestores(const WorkerInstance& worker, NeedId needId) const
	{
		if (worker.allocatedJobId == 0)
		{
			return false;
		}

		const JobInstance& job = _jobData.getJob(worker.allocatedJobId);
		if (!_jobPrototypes.isPrototypeRegistered(job.prototypeId))
		{
			return false;
		}

		return _jobPrototypes.getPrototype(job.prototypeId).needRestore.contains(needId);
	}

	void WorkerNeedService::unassign(WorkerInstance& worker)
	{
		if (worker.allocatedJobId == 0)
		{
			return;
		}

		JobInstance& job = _jobData.getJob(worker.allocatedJobId);
		job.allocatedWorkerId = 0;
		worker.allocatedJobId = 0;
		worker.state = WorkerState::Idle;
		worker.idleTime = 0.0f;
	}

	void WorkerNeedService::preemptIfNeeded(WorkerInstance& worker)
	{
		if (hasCollapsedNeed(worker))
		{
			unassign(worker);
			worker.leaving = true;
			return;
		}

		if (worker.allocatedJobId == 0)
		{
			return;
		}

		const std::optional<NeedId> chosen = chosenSeekNeed(worker);
		if (!chosen)
		{
			return;
		}

		if (!currentJobRestores(worker, *chosen))
		{
			unassign(worker);
		}
	}

	void WorkerNeedService::update(float delta)
	{
		for (WorkerInstance& worker : _workerData.workers)
		{
			for (const NeedId needId : _needPrototypes.registeredIds())
			{
				const auto it = worker.needValues.find(needId);
				if (it == worker.needValues.end())
				{
					continue;
				}

				const NeedPrototype& prototype = _needPrototypes.getPrototype(needId);
				const NeedDecayModifier modifier = decayModifier(worker, needId);
				it->second -= (prototype.decayPerSecond * modifier.multiplier + modifier.additivePerSecond) * delta;
				it->second = std::clamp(it->second, 0.0f, kNeedValueFull);
			}

			applyRestore(worker, delta);
			leaveIfRestored(worker);
			preemptIfNeeded(worker);
		}
	}

}
