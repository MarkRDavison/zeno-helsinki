#include <Services/WorkerNeedService.hpp>
#include <algorithm>

namespace drl
{

	WorkerNeedService::WorkerNeedService(
		WorkerData& workerData,
		const INeedPrototypeService& needPrototypes,
		const JobData& jobData,
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
		}
	}

}
