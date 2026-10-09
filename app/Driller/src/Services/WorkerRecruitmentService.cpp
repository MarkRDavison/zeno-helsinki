#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{

	WorkerRecruitmentService::WorkerRecruitmentService(
		WorkerData& workerData,
		const IWorkerPrototypeService& workerPrototypes)
		: _workerData(workerData)
		, _workerPrototypes(workerPrototypes)
	{
	}

	void WorkerRecruitmentService::reduceWorkerPrototypeRequirement(const std::string& prototypeName, int amount)
	{
		reduceWorkerPrototypeRequirement(_workerPrototypes.getPrototypeId(prototypeName), amount);
	}

	void WorkerRecruitmentService::reduceWorkerPrototypeRequirement(WorkerPrototypeId id, int amount)
	{
		const int existing = getRequiredWorkerCount(id);
		if (existing >= amount)
		{
			_workerData.requiredWorkers[id] -= amount;
		}
	}

	void WorkerRecruitmentService::registerWorkerPrototypeRequirement(const std::string& prototypeName, int amount)
	{
		registerWorkerPrototypeRequirement(_workerPrototypes.getPrototypeId(prototypeName), amount);
	}

	void WorkerRecruitmentService::registerWorkerPrototypeRequirement(WorkerPrototypeId id, int amount)
	{
		_workerData.requiredWorkers[id] += amount;
	}

	int WorkerRecruitmentService::getRequiredWorkerCount(const std::string& prototypeName) const
	{
		return getRequiredWorkerCount(_workerPrototypes.getPrototypeId(prototypeName));
	}

	int WorkerRecruitmentService::getRequiredWorkerCount(WorkerPrototypeId id) const
	{
		const auto it = _workerData.requiredWorkers.find(id);
		if (it == _workerData.requiredWorkers.end())
		{
			return 0;
		}
		return it->second;
	}

	std::unordered_set<WorkerPrototypeId> WorkerRecruitmentService::getRequiredWorkerTypes() const
	{
		std::unordered_set<WorkerPrototypeId> types;
		for (const auto& [id, count] : _workerData.requiredWorkers)
		{
			if (count > 0)
			{
				types.insert(id);
			}
		}
		return types;
	}

}
