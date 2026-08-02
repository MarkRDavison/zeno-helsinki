#include <Services/GameStateService.hpp>

namespace hur
{

	GameStateService::GameStateService(
	) : 
		_currentLivesRemaining(0),
		_currentScore(0)
	{

	}

	int GameStateService::getLivesRemaining() const
	{
		return _currentLivesRemaining;
	}

	void GameStateService::setLivesRemaining(int lives)
	{
		_currentLivesRemaining = lives;
	}


	int GameStateService::getScore() const
	{
		return _currentScore;
	}

	void GameStateService::setScore(int score)
	{
		_currentScore = score;
	}

	void GameStateService::incrementScore(int amount)
	{
		setScore(_currentScore + amount);
	}
}