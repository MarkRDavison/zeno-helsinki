#include <Core/BuildJobClick.hpp>
#include <Core/GameCommand.hpp>

namespace drl
{

	void enqueueBuildJob(
		IGameCommandService& commands,
		ITerrainAlterationService& terrain,
		int level,
		int column,
		const std::string& buildingPrototypeName)
	{
		if (column == 0 || buildingPrototypeName.empty())
		{
			return;
		}

		terrain.initialiseTile(level, column);
		commands.execute(GameCommand::createJob(
			"Job_Build_Building",
			buildingPrototypeName,
			level,
			column,
			CommandSource::Player,
			CommandContext::PlacingBuilding));
	}

}
