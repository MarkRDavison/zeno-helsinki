#include <Services/TowerBuildService.hpp>
#include <AudioCatalog.hpp>
#include <BoardQuery.hpp>
#include <Spawn.hpp>
#include <Components/TowerComponent.hpp>
#include <Services/GameStateService.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/TowerSelectionService.hpp>
#include <Services/WaveService.hpp>
#include <helsinki/Audio/Audio.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>

namespace tower
{
	TowerBuildService::TowerBuildService(
		hl::Scene& scene,
		WaveService& wave,
		GameStateService& gameState,
		TowerCatalog& towers,
		LevelCatalog& level,
		MatchContext& match,
		hl::audio::Audio* audio) :
		_scene(scene),
		_wave(wave),
		_gameState(gameState),
		_towers(towers),
		_level(level),
		_match(match),
		_audio(audio)
	{
	}

	void TowerBuildService::setResourceManager(hl::ResourceManager* resources)
	{
		_resources = resources;
	}

	bool TowerBuildService::tileOccupied(int tx, int tz) const
	{
		return ::tower::tileOccupied(_scene, tx, tz);
	}

	bool TowerBuildService::tryPlace(std::string_view defId, int x, int z)
	{
		if (_gameState.matchEnded() || _wave.inCombat())
		{
			return false;
		}

		const auto* def = _towers.find(defId);
		if (def == nullptr
			|| !matchAllowsTower(defId, _match)
			|| tileUnbuildable(_level, _scene, x, z))
		{
			return false;
		}

		if (!_gameState.trySpend(def->cost))
		{
			return false;
		}

		spawnTower(_scene, *def, _level, x, z, _resources);

		if (_audio != nullptr)
		{
			_audio->play(CuePlace);
		}

		return true;
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
