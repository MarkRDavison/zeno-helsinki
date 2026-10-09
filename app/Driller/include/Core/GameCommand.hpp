#pragma once

#include <string>
#include <variant>

namespace drl
{

	enum class CommandSource
	{
		Setup,
		Player,
		System
	};

	enum class CommandContext
	{
		DiggingShaft,
		DiggingTile,
		AddResource,
		Undefined
	};

	struct DigShaft
	{
		int level{ 0 };
	};

	struct DigTile
	{
		int level{ 0 };
		int column{ 0 };
	};

	struct AddResource
	{
		std::string name;
		long long amount{ 0 };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		CommandContext context{ CommandContext::Undefined };
		std::variant<DigShaft, DigTile, AddResource> payload;

		static GameCommand digShaft(int level, CommandSource source, CommandContext context)
		{
			return GameCommand{ source, context, DigShaft{ level } };
		}

		static GameCommand digTile(int level, int column, CommandSource source, CommandContext context)
		{
			return GameCommand{ source, context, DigTile{ level, column } };
		}

		static GameCommand addResource(std::string name, long long amount, CommandSource source, CommandContext context)
		{
			return GameCommand{ source, context, AddResource{ std::move(name), amount } };
		}
	};

}
