#include <Services/GameCommandService.hpp>
#include <Entities/Job.hpp>
#include <Entities/Worker.hpp>
#include <Services/PrototypeService.hpp>
#include <helsinki/System/glm.hpp>
#include <cmath>
#include <type_traits>
#include <vector>

namespace drl
{

	GameCommandService::GameCommandService(
		ITerrainAlterationService& terrain,
		IEconomyResourceService& economy,
		IJobCreationService& jobs,
		IWorkerCreationService& workers,
		IBuildingPlacementService& buildings,
		IBuildingPrototypeService& buildingPrototypes,
		IShuttleCreationService& shuttles,
		IUpgradeService& upgrades,
		WorkerData& workerData)
		: _terrain(terrain)
		, _economy(economy)
		, _jobs(jobs)
		, _workers(workers)
		, _buildings(buildings)
		, _buildingPrototypes(buildingPrototypes)
		, _shuttles(shuttles)
		, _upgrades(upgrades)
		, _workerData(workerData)
	{
	}

	bool GameCommandService::execute(const GameCommand& command)
	{
		return std::visit(
			[&](const auto& payload) -> bool
			{
				using T = std::decay_t<decltype(payload)>;
				if constexpr (std::is_same_v<T, DigShaft>)
				{
					return handleDigShaft(command.source, payload);
				}
				else if constexpr (std::is_same_v<T, DigTile>)
				{
					return handleDigTile(payload);
				}
				else if constexpr (std::is_same_v<T, AddResource>)
				{
					return handleAddResource(payload);
				}
				else if constexpr (std::is_same_v<T, CreateJob>)
				{
					return handleCreateJob(command.source, payload);
				}
				else if constexpr (std::is_same_v<T, CreateWorker>)
				{
					return handleCreateWorker(payload);
				}
				else if constexpr (std::is_same_v<T, PlaceBuilding>)
				{
					return handlePlaceBuilding(payload);
				}
				else if constexpr (std::is_same_v<T, CreateShuttle>)
				{
					return handleCreateShuttle(payload);
				}
				else if constexpr (std::is_same_v<T, AddUpgrade>)
				{
					return handleAddUpgrade(payload);
				}
				else if constexpr (std::is_same_v<T, CancelJob>)
				{
					return handleCancelJob(command.source, payload);
				}
				else
				{
					return false;
				}
			},
			command.payload);
	}

	void GameCommandService::tick()
	{
		++_tick;
	}

	long long GameCommandService::currentTick() const
	{
		return _tick;
	}

	bool GameCommandService::handleDigShaft(CommandSource source, const DigShaft& event)
	{
		if (source == CommandSource::Player)
		{
			const long long cost = kPlayerShaftDigCostPerLevel * static_cast<long long>(event.level);
			if (!_economy.canAfford(ResourceMoney, cost) ||
				!_terrain.isLevelNextShaftLevel(event.level))
			{
				return false;
			}

			if (!_economy.pay(ResourceMoney, cost))
			{
				return false;
			}
		}

		return _terrain.digShaft(event.level);
	}

	bool GameCommandService::handleDigTile(const DigTile& event)
	{
		return _terrain.digTile(event.level, event.column);
	}

	bool GameCommandService::handleAddResource(const AddResource& event)
	{
		long long amount = event.amount;
		if (event.name == ResourceOre)
		{
			amount = std::llround(
				static_cast<double>(event.amount) * static_cast<double>(_upgrades.oreMultiplier()));
		}
		_economy.add(event.name, amount);
		return true;
	}

	bool GameCommandService::handleCreateJob(CommandSource source, const CreateJob& event)
	{
		const bool playerBuild =
			source == CommandSource::Player && event.prototypeName == kJobBuildBuilding;

		long long cost = 0;
		if (playerBuild)
		{
			if (event.additionalPrototypeName.empty())
			{
				return false;
			}

			const long long buildingId = prototypeIdFromName(event.additionalPrototypeName);
			if (!_buildingPrototypes.isPrototypeRegistered(buildingId))
			{
				return false;
			}

			cost = _buildingPrototypes.getPrototype(buildingId).cost;
			if (!_economy.canAfford(ResourceMoney, cost))
			{
				return false;
			}

			if (!_economy.pay(ResourceMoney, cost))
			{
				return false;
			}
		}

		const bool created = _jobs.createJob(
			jobPrototypeIdFromName(event.prototypeName),
			jobPrototypeIdFromName(event.additionalPrototypeName),
			glm::ivec2(event.column, event.level));
		if (playerBuild && !created)
		{
			_economy.add(ResourceMoney, cost);
		}

		return created;
	}

	bool GameCommandService::handleCreateWorker(const CreateWorker& event)
	{
		return _workers.createWorker(
			prototypeIdFromName(event.prototypeName),
			event.coordinates);
	}

	bool GameCommandService::handlePlaceBuilding(const PlaceBuilding& event)
	{
		return _buildings.placePrototype(event.prototypeId, event.level, event.column);
	}

	bool GameCommandService::handleCreateShuttle(const CreateShuttle& event)
	{
		return _shuttles.createShuttle(event.prototypeId);
	}

	bool GameCommandService::handleAddUpgrade(const AddUpgrade& event)
	{
		_upgrades.addUpgrade(event.upgradeId, event.value);
		return true;
	}

	bool GameCommandService::handleCancelJob(CommandSource source, const CancelJob& event)
	{
		std::vector<JobInstance> cancelled;
		if (!_jobs.cancelNonRepeatingJobs(glm::ivec2(event.column, event.level), cancelled))
		{
			return false;
		}

		for (const JobInstance& job : cancelled)
		{
			if (job.allocatedWorkerId != 0)
			{
				for (WorkerInstance& worker : _workerData.workers)
				{
					if (worker.id == job.allocatedWorkerId)
					{
						worker.allocatedJobId = 0;
						worker.state = WorkerState::Idle;
						worker.idleTime = 0.0f;
						break;
					}
				}
			}

			if (source == CommandSource::Player
				&& job.prototypeId == jobPrototypeIdFromName(kJobBuildBuilding)
				&& _buildingPrototypes.isPrototypeRegistered(job.additionalPrototypeId))
			{
				_economy.add(
					ResourceMoney,
					_buildingPrototypes.getPrototype(job.additionalPrototypeId).cost);
			}
		}

		return true;
	}

}
