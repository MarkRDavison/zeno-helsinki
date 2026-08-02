#pragma once

namespace hur
{

	class GameStateService
	{
	public:
		GameStateService();

		int getLivesRemaining() const;
		void setLivesRemaining(int lives);

		int getScore() const;
		void setScore(int score);
		void incrementScore(int amount);

	private:
		int _currentLivesRemaining;
		int _currentScore;
	};

}