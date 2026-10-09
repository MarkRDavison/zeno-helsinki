#pragma once

#include <Entities/Worker.hpp>
#include <Services/PrototypeService.hpp>

namespace drl
{

	using IWorkerPrototypeService = IPrototypeService<WorkerInstance, WorkerPrototype>;

	class WorkerPrototypeService : public PrototypeService<WorkerInstance, WorkerPrototype>
	{
	protected:
		WorkerInstance createInstanceFromPrototype(const WorkerPrototype& prototype) override
		{
			WorkerInstance worker{};
			worker.id = allocateInstanceId();
			worker.prototypeId = prototypeIdFromName(prototype.name);
			return worker;
		}
	};

}
