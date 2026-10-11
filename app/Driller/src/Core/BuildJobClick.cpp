#include <Core/BuildJobClick.hpp>
#include <Core/GameCommand.hpp>

namespace drl
{

	bool enqueueBuildJob(
		IGameCommandService& commands,
		ITerrainAlterationService& terrain,
		int level,
		int column,
		const std::string& buildingPrototypeName)
	{
		if (column == 0 || buildingPrototypeName.empty())
		{
			return false;
		}

		terrain.initialiseTile(level, column);
		return commands.execute(GameCommand::createJob(
			kJobBuildBuilding,
			buildingPrototypeName,
			level,
			column,
			CommandSource::Player,
			CommandContext::PlacingBuilding));
	}

}
