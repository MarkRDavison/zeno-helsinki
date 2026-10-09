#pragma once

#include <Entities/Data/JobData.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <helsinki/System/glm.hpp>

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

	private:
		JobData& _jobData;
		IJobPrototypeService& _jobPrototypeService;
		ITerrainAlterationService& _terrain;
	};

}
