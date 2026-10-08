#pragma once

#include <Commands/IGameCommand.hpp>
#include <string>

namespace tower
{
	struct SelectTower : IGameCommand
	{
		int towerEntityId = -1;
	};

	struct DeselectTower : IGameCommand
	{
		bool clearFocus = false;
	};

	struct AssignTowerFocus : IGameCommand
	{
		int targetEntityId = -1;
	};

	struct PlaceTower : IGameCommand
	{
		std::string defId;
		int x = 0;
		int z = 0;
	};

	struct SellSelected : IGameCommand
	{
	};
}
