#pragma once

#include <Entities/Data/WorkerData.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <string>
#include <unordered_set>

namespace drl
{

	class IWorkerRecruitmentService
	{
	public:
		virtual ~IWorkerRecruitmentService() = 0;

		virtual void reduceWorkerPrototypeRequirement(const std::string& prototypeName, int amount) = 0;
		virtual void reduceWorkerPrototypeRequirement(WorkerPrototypeId id, int amount) = 0;
		virtual void registerWorkerPrototypeRequirement(const std::string& prototypeName, int amount) = 0;
		virtual void registerWorkerPrototypeRequirement(WorkerPrototypeId id, int amount) = 0;
		virtual int getRequiredWorkerCount(const std::string& prototypeName) const = 0;
		virtual int getRequiredWorkerCount(WorkerPrototypeId id) const = 0;
		virtual std::unordered_set<WorkerPrototypeId> getRequiredWorkerTypes() const = 0;
	};

	inline IWorkerRecruitmentService::~IWorkerRecruitmentService() = default;

	class WorkerRecruitmentService : public IWorkerRecruitmentService
	{
	public:
		WorkerRecruitmentService(WorkerData& workerData, const IWorkerPrototypeService& workerPrototypes);
		~WorkerRecruitmentService() override = default;

		void reduceWorkerPrototypeRequirement(const std::string& prototypeName, int amount) override;
		void reduceWorkerPrototypeRequirement(WorkerPrototypeId id, int amount) override;
		void registerWorkerPrototypeRequirement(const std::string& prototypeName, int amount) override;
		void registerWorkerPrototypeRequirement(WorkerPrototypeId id, int amount) override;
		int getRequiredWorkerCount(const std::string& prototypeName) const override;
		int getRequiredWorkerCount(WorkerPrototypeId id) const override;
		std::unordered_set<WorkerPrototypeId> getRequiredWorkerTypes() const override;

	private:
		WorkerData& _workerData;
		const IWorkerPrototypeService& _workerPrototypes;
	};

}
