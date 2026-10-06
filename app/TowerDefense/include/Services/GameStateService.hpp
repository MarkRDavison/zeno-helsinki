#pragma once

#include <Services/LevelCatalog.hpp>

namespace tower
{
	class GameStateService
	{
	public:
		explicit GameStateService(LevelCatalog& level);

		int gold() const;
		int lives() const;
		bool trySpend(int cost);
		void addGold(int amount);
		void onLeak();
		bool matchEnded() const;
		bool won() const;
		void setWon();

	private:
		int _gold;
		int _lives;
		bool _won = false;
		bool _gameOver = false;
	};
}
