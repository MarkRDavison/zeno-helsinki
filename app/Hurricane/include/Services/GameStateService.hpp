#pragma once

namespace hur
{

	class GameStateService
	{
	public:
		GameStateService();

		int getLivesRemaining() const;
		void setLivesRemaining(int lives);

	private:
		int _currentLivesRemaining;
	};

}