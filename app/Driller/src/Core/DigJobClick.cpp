#include <Core/DigJobClick.hpp>
#include <Core/GameCommand.hpp>

namespace drl
{

	namespace
	{
		void enqueueColumn(IGameCommandService& commands, ITerrainAlterationService& terrain, int level, int column)
		{
			if (terrain.isTileDugOut(level, column))
			{
				return;
			}

			terrain.initialiseTile(level, column);
			commands.execute(GameCommand::createJob(
				"Job_Dig",
				"",
				level,
				column,
				CommandSource::Player,
				CommandContext::CreatingJob));
		}
	}

	void enqueueDigJobs(
		IGameCommandService& commands,
		ITerrainAlterationService& terrain,
		int level,
		int column,
		bool shiftRange)
	{
		if (column == 0)
		{
			return;
		}

		const int startX = shiftRange
			? (column > 0 ? 1 : -1)
			: column;

		if (column > 0)
		{
			for (int x = startX; x <= column; ++x)
			{
				enqueueColumn(commands, terrain, level, x);
			}
		}
		else
		{
			for (int x = startX; x >= column; --x)
			{
				enqueueColumn(commands, terrain, level, x);
			}
		}
	}

}
