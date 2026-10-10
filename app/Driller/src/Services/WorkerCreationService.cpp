#include <Services/WorkerCreationService.hpp>

namespace drl
{

	WorkerCreationService::WorkerCreationService(
		WorkerData& workerData,
		IWorkerPrototypeService& workerPrototypeService,
		const BuildingData& buildingData,
		const IBuildingPrototypeService& buildingPrototypes,
		const INeedPrototypeService& needPrototypes)
		: _workerData(workerData)
		, _workerPrototypeService(workerPrototypeService)
		, _buildingData(buildingData)
		, _buildingPrototypes(buildingPrototypes)
		, _needPrototypes(needPrototypes)
	{
	}

	bool WorkerCreationService::createWorker(WorkerPrototypeId prototypeId, glm::vec2 position)
	{
		if (!_workerPrototypeService.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		if (static_cast<long long>(_workerData.workers.size())
			>= workerHousingCapacity(_buildingData, _buildingPrototypes))
		{
			return false;
		}

		WorkerInstance& worker = _workerData.workers.emplace_back(
			_workerPrototypeService.createInstance(prototypeId));
		worker.position = position;
		worker.state = WorkerState::Idle;
		for (const NeedId needId : _needPrototypes.registeredIds())
		{
			worker.needValues[needId] = kNeedValueFull;
		}
		return true;
	}

	bool WorkerCreationService::isWorkerPrototypeRegistered(WorkerPrototypeId prototypeId) const
	{
		return _workerPrototypeService.isPrototypeRegistered(prototypeId);
	}

	bool WorkerCreationService::hasSpareHousing() const
	{
		return static_cast<long long>(_workerData.workers.size())
			< workerHousingCapacity(_buildingData, _buildingPrototypes);
	}

}
