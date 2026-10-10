#pragma once

#include <Entities/Data/JobData.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <helsinki/System/glm.hpp>
#include <string>
#include <vector>

namespace drl
{

	class IJobCreationService
	{
	public:
		virtual ~IJobCreationService() = 0;

		virtual bool createJob(
			JobPrototypeId prototypeId,
			JobPrototypeId additionalPrototypeId,
			glm::ivec2 coordinates) = 0;
		virtual bool createJob(
			const std::string& prototypeName,
			glm::vec2 offset,
			glm::ivec2 coordinates) = 0;
		virtual bool isNamedPrototypeRegistered(const std::string& prototypeName) const = 0;
		virtual bool cancelNonRepeatingJobs(
			glm::ivec2 coordinates,
			std::vector<JobInstance>& cancelled) = 0;
	};

	inline IJobCreationService::~IJobCreationService() = default;

	class JobCreationService : public IJobCreationService
	{
	public:
		JobCreationService(
			JobData& jobData,
			IJobPrototypeService& jobPrototypeService,
			ITerrainAlterationService& terrain);
		~JobCreationService() override = default;

		bool createJob(
			JobPrototypeId prototypeId,
			JobPrototypeId additionalPrototypeId,
			glm::ivec2 coordinates) override;
		bool createJob(
			const std::string& prototypeName,
			glm::vec2 offset,
			glm::ivec2 coordinates) override;
		bool isNamedPrototypeRegistered(const std::string& prototypeName) const override;
		bool cancelNonRepeatingJobs(
			glm::ivec2 coordinates,
			std::vector<JobInstance>& cancelled) override;

	private:
		JobData& _jobData;
		IJobPrototypeService& _jobPrototypeService;
		ITerrainAlterationService& _terrain;
	};

}
