#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <unordered_set>

namespace drl
{

	using WorkerId = long long;
	using WorkerPrototypeId = long long;

	enum class WorkerState
	{
		Idle,
		MovingToJob,
		WorkingJob,
		Wander
	};

	struct WorkerInstance
	{
		WorkerId id{ 0 };
		WorkerPrototypeId prototypeId{ 0 };
		long long allocatedJobId{ 0 };
		glm::vec2 position{ 0.0f, 0.0f };
		WorkerState state{ WorkerState::Idle };
		glm::vec2 wanderTarget{ 0.0f, 0.0f };
		float idleTime{ 0.0f };
		float wanderBackoff{ 0.0f };
	};

	struct WorkerPrototype
	{
		std::string name;
		std::unordered_set<std::string> validJobPrototypes;
	};

}
