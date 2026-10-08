#include <Services/TowerFocusService.hpp>
#include <Components/TowerComponent.hpp>
#include <Services/GameStateService.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/TowerSelectionService.hpp>
#include <Targeting.hpp>

namespace tower
{
	TowerFocusService::TowerFocusService(
		hl::Scene& scene,
		TowerSelectionService& selection,
		GameStateService& gameState,
		TowerCatalog& towers,
		LevelCatalog& level) :
		_scene(scene),
		_selection(selection),
		_gameState(gameState),
		_towers(towers),
		_level(level)
	{
	}

	bool TowerFocusService::tryAssign(int targetEntityId)
	{
		if (_gameState.matchEnded())
		{
			return false;
		}

		auto* selected = _selection.selected();
		auto* tower = (selected != nullptr) ? selected->GetComponent<TowerComponent>() : nullptr;
		const auto* def = (tower != nullptr) ? _towers.find(tower->defId) : nullptr;
		if (tower == nullptr || def == nullptr)
		{
			return false;
		}

		auto* target = _scene.getEntity(targetEntityId);
		const glm::vec3 from = _level.tileCenter(tower->x, tower->z);
		return tryAssignTowerFocus(_scene, from, def->range, target, tower->focusEntityId);
	}
}
