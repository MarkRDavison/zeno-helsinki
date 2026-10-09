#include <Services/JobPrototypeService.hpp>

namespace drl
{

	JobInstance JobPrototypeService::createInstance(long long prototypeId)
	{
		JobInstance job = PrototypeService::createInstance(prototypeId);
		job.work = getPrototype(prototypeId).work;
		return job;
	}

}
