#pragma once

#include <helsinki/System/glm.hpp>
#include <string>
#include <utility>
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
		CreatingJob,
		CreatingWorker,
		Undefined
	};

	struct DigShaft
	{
		DigShaft() = default;
		explicit DigShaft(int level) : level(level) {}

		int level{ 0 };
	};

	struct DigTile
	{
		DigTile() = default;
		DigTile(int level, int column) : level(level), column(column) {}

		int level{ 0 };
		int column{ 0 };
	};

	struct AddResource
	{
		AddResource() = default;
		AddResource(std::string name, long long amount)
			: name(std::move(name))
			, amount(amount)
		{
		}

		std::string name;
		long long amount{ 0 };
	};

	struct CreateJob
	{
		CreateJob() = default;
		CreateJob(std::string prototypeName, std::string additionalPrototypeName, int level, int column)
			: prototypeName(std::move(prototypeName))
			, additionalPrototypeName(std::move(additionalPrototypeName))
			, level(level)
			, column(column)
		{
		}

		std::string prototypeName;
		std::string additionalPrototypeName;
		int level{ 0 };
		int column{ 0 };
	};

	struct CreateWorker
	{
		CreateWorker() = default;
		CreateWorker(std::string prototypeName, glm::vec2 coordinates)
			: prototypeName(std::move(prototypeName))
			, coordinates(coordinates)
		{
		}

		std::string prototypeName;
		glm::vec2 coordinates{ 0.0f, 0.0f };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		CommandContext context{ CommandContext::Undefined };
		std::variant<DigShaft, DigTile, AddResource, CreateJob, CreateWorker> payload;

		GameCommand() = default;

		GameCommand(const DigShaft& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		GameCommand(const DigTile& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		GameCommand(const AddResource& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		GameCommand(const CreateJob& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		GameCommand(const CreateWorker& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		static GameCommand digShaft(int level, CommandSource commandSource, CommandContext commandContext)
		{
			return GameCommand(DigShaft{ level }, commandContext, commandSource);
		}

		static GameCommand digTile(int level, int column, CommandSource commandSource, CommandContext commandContext)
		{
			return GameCommand(DigTile{ level, column }, commandContext, commandSource);
		}

		static GameCommand addResource(std::string name, long long amount, CommandSource commandSource, CommandContext commandContext)
		{
			return GameCommand(AddResource{ std::move(name), amount }, commandContext, commandSource);
		}

		static GameCommand createJob(
			std::string prototypeName,
			std::string additionalPrototypeName,
			int level,
			int column,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				CreateJob{ std::move(prototypeName), std::move(additionalPrototypeName), level, column },
				commandContext,
				commandSource);
		}

		static GameCommand createWorker(
			std::string prototypeName,
			glm::vec2 coordinates,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				CreateWorker{ std::move(prototypeName), coordinates },
				commandContext,
				commandSource);
		}
	};

}
