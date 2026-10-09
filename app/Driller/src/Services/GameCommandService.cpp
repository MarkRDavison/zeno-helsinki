#include <Services/GameCommandService.hpp>
#include <Entities/Job.hpp>
#include <Services/PrototypeService.hpp>
#include <helsinki/System/glm.hpp>
#include <type_traits>

namespace drl
{

	GameCommandService::GameCommandService(
		ITerrainAlterationService& terrain,
		IEconomyResourceService& economy,
		IJobCreationService& jobs,
		IWorkerCreationService& workers)
		: _terrain(terrain)
		, _economy(economy)
		, _jobs(jobs)
		, _workers(workers)
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
					return handleCreateJob(payload);
				}
				else if constexpr (std::is_same_v<T, CreateWorker>)
				{
					return handleCreateWorker(payload);
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
		_economy.add(event.name, event.amount);
		return true;
	}

	bool GameCommandService::handleCreateJob(const CreateJob& event)
	{
		return _jobs.createJob(
			jobPrototypeIdFromName(event.prototypeName),
			jobPrototypeIdFromName(event.additionalPrototypeName),
			glm::ivec2(event.column, event.level));
	}

	bool GameCommandService::handleCreateWorker(const CreateWorker& event)
	{
		return _workers.createWorker(
			prototypeIdFromName(event.prototypeName),
			event.coordinates);
	}

}
