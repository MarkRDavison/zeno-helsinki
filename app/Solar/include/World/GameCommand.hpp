#pragma once

#include <World/WorldData.hpp>
#include <variant>

namespace slr
{

	enum class CommandSource
	{
		Player,
		Ai
	};

	struct MoveFleetToSystem
	{
		long long fleetId{ 0 };
		long long systemId{ 0 };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		std::variant<MoveFleetToSystem> payload;

		static GameCommand moveFleetToSystem(
			long long fleetId,
			long long systemId,
			CommandSource source);
	};

	bool execute(WorldData& world, const GameCommand& command);

}
