#pragma once

#include <Entities/Worker.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace drl
{

	struct WorkerData
	{
		std::vector<WorkerInstance> workers;

		WorkerInstance& getWorker(WorkerId workerId)
		{
			const auto workerIter = std::find_if(
				workers.begin(),
				workers.end(),
				[workerId](const WorkerInstance& worker)
				{
					return worker.id == workerId;
				});
			if (workerIter == workers.end())
			{
				throw std::logic_error("Invalid worker id");
			}
			return *workerIter;
		}

		const WorkerInstance& getWorker(WorkerId workerId) const
		{
			const auto workerIter = std::find_if(
				workers.begin(),
				workers.end(),
				[workerId](const WorkerInstance& worker)
				{
					return worker.id == workerId;
				});
			if (workerIter == workers.end())
			{
				throw std::logic_error("Invalid worker id");
			}
			return *workerIter;
		}
	};

}
