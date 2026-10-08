#pragma once

#include <Commands/GameCommands.hpp>

namespace tower
{
	class TowerSelectionService;
	class TowerFocusService;
	class TowerBuildService;

	class GameCommandService
	{
	public:
		GameCommandService(
			TowerSelectionService& selection,
			TowerFocusService& focus,
			TowerBuildService& build);

		bool handle(const SelectTower& command);
		bool handle(const DeselectTower& command);
		bool handle(const AssignTowerFocus& command);
		bool handle(const PlaceTower& command);
		bool handle(const SellSelected& command);

	private:
		TowerSelectionService& _selection;
		TowerFocusService& _focus;
		TowerBuildService& _build;
	};
}
