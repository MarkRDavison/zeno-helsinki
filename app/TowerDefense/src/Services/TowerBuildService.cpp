#include <Services/TowerBuildService.hpp>
#include <Components/EntityComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <SceneCatalog.hpp>
#include <Services/GameStateService.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/TowerSelectionService.hpp>
#include <Services/WaveService.hpp>

namespace tower
{
	TowerBuildService::TowerBuildService(
		hl::Scene& scene,
		WaveService& wave,
		GameStateService& gameState,
		TowerCatalog& towers,
		LevelCatalog& level) :
		_scene(scene),
		_wave(wave),
		_gameState(gameState),
		_towers(towers),
		_level(level)
	{
	}

	bool TowerBuildService::tileOccupied(int tx, int tz) const
	{
		for (auto* entity : _scene.getEntitiesByTag(TowerTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			const auto* tower = entity->GetComponent<TowerComponent>();
			if (tower != nullptr && tower->x == tx && tower->z == tz)
			{
				return true;
			}
		}

		for (auto* entity : _scene.getEntitiesWithComponents<EntityComponent>())
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			const auto* placed = entity->GetComponent<EntityComponent>();
			if (placed != nullptr
				&& tx >= placed->x && tx < placed->x + placed->sizeX
				&& tz >= placed->z && tz < placed->z + placed->sizeZ)
			{
				return true;
			}
		}

		return false;
	}

	bool TowerBuildService::tryPlace(std::string_view defId, int x, int z)
	{
		if (_gameState.matchEnded() || _wave.inCombat())
		{
			return false;
		}

		const auto* def = _towers.find(defId);
		if (def == nullptr
			|| !_level.isOnBoard(x, z)
			|| _level.isPathTile(x, z)
			|| tileOccupied(x, z))
		{
			return false;
		}

		return _gameState.trySpend(def->cost);
	}

	bool TowerBuildService::trySell(TowerSelectionService& selection)
	{
		if (_gameState.matchEnded() || _wave.inCombat())
		{
			return false;
		}

		auto* entity = selection.selected();
		auto* tower = (entity != nullptr) ? entity->GetComponent<TowerComponent>() : nullptr;
		if (tower == nullptr)
		{
			return false;
		}

		const auto* def = _towers.find(tower->defId);
		const int cost = def != nullptr ? def->cost : 0;
		_gameState.addGold(towerSellRefund(cost));
		const int id = entity->Id;
		_gameState.setSelectedTowerId(NoTowerSelected);
		_scene.removeEntity(id);
		return true;
	}
}
