#include <Services/GameStateService.hpp>
#include <SceneCatalog.hpp>

namespace tower
{
	GameStateService::GameStateService() :
		_gold(StartGold),
		_lives(StartLives)
	{
	}

	int GameStateService::gold() const
	{
		return _gold;
	}

	int GameStateService::lives() const
	{
		return _lives;
	}

	bool GameStateService::trySpend(int cost)
	{
		if (matchEnded() || _gold < cost)
		{
			return false;
		}

		_gold -= cost;
		return true;
	}

	void GameStateService::addGold(int amount)
	{
		if (matchEnded())
		{
			return;
		}

		_gold += amount;
	}

	void GameStateService::onLeak()
	{
		if (matchEnded())
		{
			return;
		}

		if (_lives > 0)
		{
			_lives -= 1;
		}

		if (_lives <= 0)
		{
			_gameOver = true;
		}
	}

	bool GameStateService::matchEnded() const
	{
		return _gameOver || _won;
	}

	bool GameStateService::won() const
	{
		return _won;
	}

	void GameStateService::setWon()
	{
		if (_gameOver)
		{
			return;
		}

		_won = true;
	}
}
