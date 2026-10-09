#include <Services/ShuttleScheduleService.hpp>
#include <helsinki/System/glm.hpp>
#include <stdexcept>

namespace drl
{

	ShuttleScheduleService::ShuttleScheduleService(
		ShuttleData& shuttleData,
		IWorkerRecruitmentService& recruitment,
		IWorkerCreationService& workerCreation,
		const IShuttlePrototypeService& shuttlePrototypes,
		IEconomyResourceService& economy)
		: _shuttleData(shuttleData)
		, _recruitment(recruitment)
		, _workerCreation(workerCreation)
		, _shuttlePrototypes(shuttlePrototypes)
		, _economy(economy)
	{
	}

	void ShuttleScheduleService::update(float delta)
	{
		for (ShuttleInstance& shuttle : _shuttleData.shuttles)
		{
			updateShuttle(delta, shuttle, _shuttlePrototypes.getPrototype(shuttle.prototypeId));
		}
	}

	void ShuttleScheduleService::updateShuttle(float delta, ShuttleInstance& shuttle, const ShuttlePrototype& prototype)
	{
		shuttle.elapsed += delta;
		switch (shuttle.state)
		{
		case ShuttleState::Idle:
			if (shuttle.elapsed >= prototype.idleTime)
			{
				shuttle.state = ShuttleState::TravellingToSurface;
				shuttle.elapsed = 0.0f;
			}
			break;
		case ShuttleState::TravellingToSurface:
			if (moveShuttleTowardsLocation(
					delta,
					shuttle,
					prototype,
					shuttle.surfacePosition,
					ShuttleState::WaitingOnSurface))
			{
				updateShuttleOnArrivalAtSurface(shuttle, prototype);
			}
			break;
		case ShuttleState::WaitingOnSurface:
			if (shuttle.elapsed >= prototype.loadingTime)
			{
				shuttle.state = ShuttleState::LeavingSurface;
				shuttle.elapsed = 0.0f;
			}
			break;
		case ShuttleState::LeavingSurface:
			moveShuttleTowardsLocation(
				delta,
				shuttle,
				prototype,
				shuttle.leavingPosition,
				ShuttleState::Completed);
			break;
		case ShuttleState::Completed:
			updateShuttleOnArrivalAtDepartureDestination(shuttle);
			break;
		default:
			throw std::runtime_error("Invalid shuttle state");
		}
	}

	bool ShuttleScheduleService::moveShuttleTowardsLocation(
		float delta,
		ShuttleInstance& shuttle,
		const ShuttlePrototype& prototype,
		glm::vec2 target,
		ShuttleState nextState)
	{
		const float distanceToTarget = glm::length(target - shuttle.position);
		const float maxMovement = prototype.speed * delta;
		if (distanceToTarget <= maxMovement)
		{
			shuttle.state = nextState;
			shuttle.position = target;
			shuttle.elapsed = 0.0f;
			return true;
		}

		shuttle.position += glm::normalize(target - shuttle.position) * maxMovement;
		return false;
	}

	void ShuttleScheduleService::updateShuttleOnArrivalAtSurface(
		ShuttleInstance& shuttle,
		const ShuttlePrototype& prototype)
	{
		const auto requiredTypes = _recruitment.getRequiredWorkerTypes();
		for (const WorkerPrototypeId prototypeId : requiredTypes)
		{
			const int amountRequired = _recruitment.getRequiredWorkerCount(prototypeId);
			_recruitment.reduceWorkerPrototypeRequirement(prototypeId, amountRequired);
			for (int i = 0; i < amountRequired; ++i)
			{
				if (!_workerCreation.createWorker(prototypeId, shuttle.position))
				{
					_recruitment.registerWorkerPrototypeRequirement(prototypeId, 1);
				}
			}
		}

		for (const std::string& resourceName : prototype.allowedCargo)
		{
			if (!_economy.exists(resourceName))
			{
				continue;
			}

			shuttle.cargo[resourceName] = _economy.get(resourceName);
			_economy.set(resourceName, 0);
		}
	}

	void ShuttleScheduleService::updateShuttleOnArrivalAtDepartureDestination(ShuttleInstance& shuttle)
	{
		shuttle.state = ShuttleState::Idle;
		shuttle.position = shuttle.startingPosition;
		shuttle.elapsed = 0.0f;

		for (const auto& cargoEntry : shuttle.cargo)
		{
			_economy.add(ResourceMoney, cargoEntry.second);
		}

		shuttle.cargo.clear();
	}

}
