#pragma once

#include <Entities/Job.hpp>
#include <Services/PrototypeService.hpp>

namespace drl
{

	using IJobPrototypeService = IPrototypeService<JobInstance, JobPrototype>;

	class JobPrototypeService : public PrototypeService<JobInstance, JobPrototype>
	{
	protected:
		JobInstance createInstanceFromPrototype(const JobPrototype& prototype) override;
	};

}
