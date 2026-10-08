#pragma once

#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace tower
{
	class GameStateService;

	class TowerSelectionService
	{
	public:
		TowerSelectionService(hl::Scene& scene, GameStateService& gameState);

		hl::Entity* selected() const;
		bool select(int towerEntityId);
		bool deselect(bool clearFocus);

	private:
		hl::Entity* towerEntity(int id) const;

		hl::Scene& _scene;
		GameStateService& _gameState;
	};
}
