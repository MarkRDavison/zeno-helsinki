#include <Services/GameCommandService.hpp>
#include <type_traits>

namespace drl
{

	GameCommandService::GameCommandService(
		ITerrainAlterationService& terrain,
		IEconomyResourceService& economy)
		: _terrain(terrain)
		, _economy(economy)
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

}
