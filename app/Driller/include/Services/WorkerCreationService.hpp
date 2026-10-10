#pragma once

#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	class IWorkerCreationService
	{
	public:
		virtual ~IWorkerCreationService() = 0;

		virtual bool createWorker(WorkerPrototypeId prototypeId, glm::vec2 position) = 0;
		virtual bool isWorkerPrototypeRegistered(WorkerPrototypeId prototypeId) const = 0;
		virtual bool hasSpareHousing() const = 0;
	};

	inline IWorkerCreationService::~IWorkerCreationService() = default;

	class WorkerCreationService : public IWorkerCreationService
	{
	public:
		WorkerCreationService(
			WorkerData& workerData,
			IWorkerPrototypeService& workerPrototypeService,
			const BuildingData& buildingData,
			const IBuildingPrototypeService& buildingPrototypes,
			const INeedPrototypeService& needPrototypes);
		~WorkerCreationService() override = default;

		bool createWorker(WorkerPrototypeId prototypeId, glm::vec2 position) override;
		bool isWorkerPrototypeRegistered(WorkerPrototypeId prototypeId) const override;
		bool hasSpareHousing() const override;

	private:
		WorkerData& _workerData;
		IWorkerPrototypeService& _workerPrototypeService;
		const BuildingData& _buildingData;
		const IBuildingPrototypeService& _buildingPrototypes;
		const INeedPrototypeService& _needPrototypes;
	};

}
