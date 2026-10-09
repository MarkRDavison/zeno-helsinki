#include <Core/SeedStartingCavern.hpp>

namespace drl
{

	void seedStartingCavern(TerrainAlterationService& terrain)
	{
		terrain.digShaft(0);
		terrain.digShaft(1);

		terrain.digTile(0, -1);
		terrain.digTile(0, -2);
		terrain.digTile(0, 1);
		terrain.digTile(0, 2);
		terrain.digTile(0, 3);
		terrain.initialiseTile(0, -3);
		terrain.initialiseTile(0, 4);

		terrain.digTile(1, -1);
		terrain.digTile(1, 1);
		terrain.digTile(1, 2);
		terrain.initialiseTile(1, -2);
		terrain.initialiseTile(1, 3);
	}

}
