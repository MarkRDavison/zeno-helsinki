#pragma once

#include <Entities/Job.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace drl
{

	struct JobData
	{
		std::vector<JobInstance> jobs;

		JobInstance& getJob(JobId jobId)
		{
			const auto jobIter = std::find_if(
				jobs.begin(),
				jobs.end(),
				[jobId](const JobInstance& job)
				{
					return job.id == jobId;
				});
			if (jobIter == jobs.end())
			{
				throw std::logic_error("Invalid job id");
			}
			return *jobIter;
		}

		const JobInstance& getJob(JobId jobId) const
		{
			const auto jobIter = std::find_if(
				jobs.begin(),
				jobs.end(),
				[jobId](const JobInstance& job)
				{
					return job.id == jobId;
				});
			if (jobIter == jobs.end())
			{
				throw std::logic_error("Invalid job id");
			}
			return *jobIter;
		}
	};

}
