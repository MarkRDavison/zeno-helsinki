#include <Scripting/CommandBindings.hpp>
#include <Core/GameCommand.hpp>
#include <string>

namespace drl
{

	void bindGameCommands(sol::state& lua, IGameCommandService& commands)
	{
		lua.new_enum<CommandContext>(
			"GameCommandContext",
			{
				{ "DiggingShaft", CommandContext::DiggingShaft },
				{ "DiggingTile", CommandContext::DiggingTile },
				{ "AddResource", CommandContext::AddResource },
				{ "CreatingJob", CommandContext::CreatingJob },
				{ "CreatingWorker", CommandContext::CreatingWorker },
				{ "Undefined", CommandContext::Undefined },
			});

		lua.new_enum<CommandSource>(
			"GameCommandSource",
			{
				{ "Setup", CommandSource::Setup },
				{ "Player", CommandSource::Player },
				{ "System", CommandSource::System },
			});

		lua.new_usertype<DigShaft>(
			"DigShaftEvent",
			sol::constructors<DigShaft(int)>(),
			"level", &DigShaft::level);

		lua.new_usertype<DigTile>(
			"DigTileEvent",
			sol::constructors<DigTile(int, int)>(),
			"level", &DigTile::level,
			"column", &DigTile::column);

		lua.new_usertype<AddResource>(
			"AddResourceEvent",
			sol::constructors<AddResource(std::string, long long)>(),
			"name", &AddResource::name,
			"amount", &AddResource::amount);

		lua.new_usertype<CreateJob>(
			"CreateJobEvent",
			sol::constructors<CreateJob(std::string, std::string, int, int)>(),
			"prototypeName", &CreateJob::prototypeName,
			"additionalPrototypeName", &CreateJob::additionalPrototypeName,
			"level", &CreateJob::level,
			"column", &CreateJob::column);

		lua.new_usertype<CreateWorker>(
			"CreateWorkerEvent",
			sol::constructors<CreateWorker(std::string, glm::vec2)>(),
			"prototypeName", &CreateWorker::prototypeName,
			"coordinates", &CreateWorker::coordinates);

		lua.new_usertype<GameCommand>(
			"GameCommand",
			sol::constructors<
				GameCommand(const DigShaft&, CommandContext, CommandSource),
				GameCommand(const DigTile&, CommandContext, CommandSource),
				GameCommand(const AddResource&, CommandContext, CommandSource),
				GameCommand(const CreateJob&, CommandContext, CommandSource),
				GameCommand(const CreateWorker&, CommandContext, CommandSource)
			>());

		lua.set_function(
			"cmd",
			[&commands](const GameCommand& command)
			{
				return commands.execute(command);
			});
	}

}
