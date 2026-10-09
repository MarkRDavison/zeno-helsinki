#pragma once

#include <Services/GameCommandService.hpp>
#include <Services/TerrainAlterationService.hpp>

namespace drl
{

	void enqueueDigJobs(
		IGameCommandService& commands,
		ITerrainAlterationService& terrain,
		int level,
		int column,
		bool shiftRange);

}
