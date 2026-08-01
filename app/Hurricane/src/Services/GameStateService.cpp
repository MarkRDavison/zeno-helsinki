#include <Services/GameStateService.hpp>

namespace hur
{

	GameStateService::GameStateService(
	) : 
		_currentLivesRemaining(0)
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
}