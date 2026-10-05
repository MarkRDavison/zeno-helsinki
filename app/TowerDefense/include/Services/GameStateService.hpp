#pragma once

namespace tower
{
	class GameStateService
	{
	public:
		GameStateService();

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
