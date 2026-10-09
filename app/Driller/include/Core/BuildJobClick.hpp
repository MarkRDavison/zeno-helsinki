#pragma once

#include <Services/GameCommandService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <string>

namespace drl
{

	void enqueueBuildJob(
		IGameCommandService& commands,
		ITerrainAlterationService& terrain,
		int level,
		int column,
		const std::string& buildingPrototypeName);

}
