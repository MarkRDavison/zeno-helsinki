#pragma once

#include <Services/GameCommandService.hpp>
#include <vector>

namespace drl
{

	class IGameTickService
	{
	public:
		virtual ~IGameTickService() = 0;
		virtual void update(float delta) = 0;
	};

	inline IGameTickService::~IGameTickService() = default;

	class Game
	{
	public:
		Game(IGameCommandService& commands, const float& simSpeed);

		void addTickService(IGameTickService& service);
		void update(float delta);

	private:
		IGameCommandService& _commands;
		const float& _simSpeed;
		std::vector<IGameTickService*> _tickServices;
	};

}
