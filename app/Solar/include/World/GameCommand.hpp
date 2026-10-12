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

	struct AcceptHaulJob
	{
		long long truckId{ 0 };
		long long jobId{ 0 };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		std::variant<MoveFleetToSystem, AcceptHaulJob> payload;

		static GameCommand moveFleetToSystem(
			long long fleetId,
			long long systemId,
			CommandSource source);
		static GameCommand acceptHaulJob(
			long long truckId,
			long long jobId,
			CommandSource source);
	};

	bool execute(WorldData& world, const GameCommand& command);

}
