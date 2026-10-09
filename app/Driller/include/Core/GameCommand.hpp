#pragma once

#include <Services/PrototypeService.hpp>
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
		PlacingBuilding,
		CreatingShuttle,
		AddingUpgrade,
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

	struct PlaceBuilding
	{
		PlaceBuilding() = default;
		PlaceBuilding(std::string prototypeName, int level, int column)
			: prototypeId(prototypeIdFromName(prototypeName))
			, level(level)
			, column(column)
		{
		}

		PlaceBuilding(long long prototypeId, int level, int column)
			: prototypeId(prototypeId)
			, level(level)
			, column(column)
		{
		}

		long long prototypeId{ 0 };
		int level{ 0 };
		int column{ 0 };
	};

	struct CreateShuttle
	{
		CreateShuttle() = default;
		explicit CreateShuttle(std::string prototypeName)
			: prototypeId(prototypeIdFromName(prototypeName))
		{
		}

		explicit CreateShuttle(long long prototypeId)
			: prototypeId(prototypeId)
		{
		}

		long long prototypeId{ 0 };
	};

	struct AddUpgrade
	{
		AddUpgrade() = default;
		AddUpgrade(std::string upgradeName, float value)
			: upgradeId(prototypeIdFromName(upgradeName))
			, value(value)
		{
		}

		AddUpgrade(long long upgradeId, float value)
			: upgradeId(upgradeId)
			, value(value)
		{
		}

		long long upgradeId{ 0 };
		float value{ 0.0f };
	};

	struct GameCommand
	{
		CommandSource source{ CommandSource::Player };
		CommandContext context{ CommandContext::Undefined };
		std::variant<DigShaft, DigTile, AddResource, CreateJob, CreateWorker, PlaceBuilding, CreateShuttle, AddUpgrade> payload;

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

		GameCommand(const PlaceBuilding& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		GameCommand(const CreateShuttle& event, CommandContext commandContext, CommandSource commandSource)
			: source(commandSource)
			, context(commandContext)
			, payload(event)
		{
		}

		GameCommand(const AddUpgrade& event, CommandContext commandContext, CommandSource commandSource)
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

		static GameCommand placeBuilding(
			std::string prototypeName,
			int level,
			int column,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				PlaceBuilding{ std::move(prototypeName), level, column },
				commandContext,
				commandSource);
		}

		static GameCommand placeBuilding(
			long long prototypeId,
			int level,
			int column,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				PlaceBuilding{ prototypeId, level, column },
				commandContext,
				commandSource);
		}

		static GameCommand createShuttle(
			std::string prototypeName,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				CreateShuttle{ std::move(prototypeName) },
				commandContext,
				commandSource);
		}

		static GameCommand createShuttle(
			long long prototypeId,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				CreateShuttle{ prototypeId },
				commandContext,
				commandSource);
		}

		static GameCommand addUpgrade(
			std::string upgradeName,
			float value,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				AddUpgrade{ std::move(upgradeName), value },
				commandContext,
				commandSource);
		}

		static GameCommand addUpgrade(
			long long upgradeId,
			float value,
			CommandSource commandSource,
			CommandContext commandContext)
		{
			return GameCommand(
				AddUpgrade{ upgradeId, value },
				commandContext,
				commandSource);
		}
	};

}
