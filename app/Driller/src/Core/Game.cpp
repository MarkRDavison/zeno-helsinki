#include <Core/Game.hpp>

namespace drl
{

	Game::Game(IGameCommandService& commands, const float& simSpeed)
		: _commands(commands)
		, _simSpeed(simSpeed)
	{
	}

	void Game::addTickService(IGameTickService& service)
	{
		_tickServices.push_back(&service);
	}

	void Game::update(float delta)
	{
		const float scaledDelta = delta * _simSpeed;
		for (IGameTickService* service : _tickServices)
		{
			service->update(scaledDelta);
		}
		_commands.tick();
	}

}
