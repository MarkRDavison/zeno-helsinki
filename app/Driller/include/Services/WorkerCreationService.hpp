#pragma once

#include <Entities/Data/WorkerData.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	class IWorkerCreationService
	{
	public:
		virtual ~IWorkerCreationService() = 0;

		virtual bool createWorker(WorkerPrototypeId prototypeId, glm::vec2 position) = 0;
	};

	inline IWorkerCreationService::~IWorkerCreationService() = default;

	class WorkerCreationService : public IWorkerCreationService
	{
	public:
		WorkerCreationService(
			WorkerData& workerData,
			IWorkerPrototypeService& workerPrototypeService);
		~WorkerCreationService() override = default;

		bool createWorker(WorkerPrototypeId prototypeId, glm::vec2 position) override;

	private:
		WorkerData& _workerData;
		IWorkerPrototypeService& _workerPrototypeService;
	};

}
