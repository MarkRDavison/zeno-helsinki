#include <Services/WorkerCreationService.hpp>

namespace drl
{

	WorkerCreationService::WorkerCreationService(
		WorkerData& workerData,
		IWorkerPrototypeService& workerPrototypeService)
		: _workerData(workerData)
		, _workerPrototypeService(workerPrototypeService)
	{
	}

	bool WorkerCreationService::createWorker(WorkerPrototypeId prototypeId, glm::vec2 position)
	{
		if (!_workerPrototypeService.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		WorkerInstance& worker = _workerData.workers.emplace_back(
			_workerPrototypeService.createInstance(prototypeId));
		worker.position = position;
		worker.state = WorkerState::Idle;
		return true;
	}

}
