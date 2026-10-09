#pragma once

#include <Entities/Job.hpp>
#include <Services/PrototypeService.hpp>

namespace drl
{

	using IJobPrototypeService = IPrototypeService<JobInstance, JobPrototype>;

	class JobPrototypeService : public PrototypeService<JobInstance, JobPrototype>
	{
	public:
		JobInstance createInstance(long long prototypeId) override;
	};

}
