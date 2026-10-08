#include <Services/TowerSelectionService.hpp>
#include <Components/TowerComponent.hpp>
#include <Services/GameStateService.hpp>

namespace tower
{
	TowerSelectionService::TowerSelectionService(hl::Scene& scene, GameStateService& gameState) :
		_scene(scene),
		_gameState(gameState)
	{
	}

	hl::Entity* TowerSelectionService::selected() const
	{
		return towerEntity(_gameState.selectedTowerId());
	}

	hl::Entity* TowerSelectionService::towerEntity(int id) const
	{
		if (id == NoTowerSelected || _scene.isPendingRemoval(id))
		{
			return nullptr;
		}

		auto* entity = _scene.getEntity(id);
		if (entity == nullptr || entity->GetComponent<TowerComponent>() == nullptr)
		{
			return nullptr;
		}

		return entity;
	}

	bool TowerSelectionService::select(int towerEntityId)
	{
		auto* entity = towerEntity(towerEntityId);
		if (entity == nullptr)
		{
			return false;
		}

		_gameState.setSelectedTowerId(entity->Id);
		return true;
	}

	bool TowerSelectionService::deselect(bool clearFocus)
	{
		auto* entity = selected();
		if (entity == nullptr)
		{
			_gameState.setSelectedTowerId(NoTowerSelected);
			return false;
		}

		if (clearFocus)
		{
			entity->GetComponent<TowerComponent>()->focusEntityId = NoTowerSelected;
		}

		_gameState.setSelectedTowerId(NoTowerSelected);
		return true;
	}
}
