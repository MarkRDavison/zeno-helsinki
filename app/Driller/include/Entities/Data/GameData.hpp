#pragma once

#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>

namespace drl
{

	struct GameData
	{
		TerrainData terrain;
		JobData job;
		WorkerData worker;
		BuildingData building;
		ShuttleData shuttle;
	};

}
