#pragma once

#include <Services/LevelCatalog.hpp>
#include <Services/MatchContext.hpp>

namespace tower
{
	inline constexpr int NoTowerSelected = -1;

	class GameStateService
	{
	public:
		GameStateService(LevelCatalog& level, MatchContext& match);

		int gold() const;
		int lives() const;
		int selectedTowerId() const;
		void setSelectedTowerId(int id);
		bool trySpend(int cost);
		void addGold(int amount);
		void onLeak();
		bool matchEnded() const;
		bool won() const;
		void setWon();

	private:
		int _gold;
		int _lives;
		int _selectedTowerId = NoTowerSelected;
		bool _won = false;
		bool _gameOver = false;
	};
}
