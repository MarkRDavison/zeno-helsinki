#include <Services/JobPrototypeService.hpp>

namespace drl
{

	JobInstance JobPrototypeService::createInstanceFromPrototype(const JobPrototype& prototype)
	{
		JobInstance job{};
		job.id = allocateInstanceId();
		job.prototypeId = prototypeIdFromName(prototype.name);
		job.work = prototype.work;
		return job;
	}

}
