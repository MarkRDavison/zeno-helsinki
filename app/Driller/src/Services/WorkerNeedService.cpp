#include <Services/WorkerNeedService.hpp>
#include <algorithm>

namespace drl
{

	WorkerNeedService::WorkerNeedService(WorkerData& workerData, const INeedPrototypeService& needPrototypes)
		: _workerData(workerData)
		, _needPrototypes(needPrototypes)
	{
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
				it->second -= prototype.decayPerSecond * delta;
				it->second = std::clamp(it->second, 0.0f, kNeedValueFull);
			}
		}
	}

}
