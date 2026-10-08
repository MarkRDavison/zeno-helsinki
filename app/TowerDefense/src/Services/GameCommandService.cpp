#include <Services/GameCommandService.hpp>
#include <Services/TowerBuildService.hpp>
#include <Services/TowerFocusService.hpp>
#include <Services/TowerSelectionService.hpp>

namespace tower
{
	GameCommandService::GameCommandService(
		TowerSelectionService& selection,
		TowerFocusService& focus,
		TowerBuildService& build) :
		_selection(selection),
		_focus(focus),
		_build(build)
	{
	}

	bool GameCommandService::handle(const SelectTower& command)
	{
		return _selection.select(command.towerEntityId);
	}

	bool GameCommandService::handle(const DeselectTower& command)
	{
		return _selection.deselect(command.clearFocus);
	}

	bool GameCommandService::handle(const AssignTowerFocus& command)
	{
		return _focus.tryAssign(command.targetEntityId);
	}

	bool GameCommandService::handle(const PlaceTower& command)
	{
		return _build.tryPlace(command.defId, command.x, command.z);
	}

	bool GameCommandService::handle(const SellSelected& command)
	{
		(void)command;
		return _build.trySell(_selection);
	}
}
