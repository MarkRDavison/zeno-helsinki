#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/WorkerData.hpp>
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
		WorkerNeedService(WorkerData& workerData, const INeedPrototypeService& needPrototypes);
		~WorkerNeedService() override = default;

		void update(float delta) override;

	private:
		WorkerData& _workerData;
		const INeedPrototypeService& _needPrototypes;
	};

}
