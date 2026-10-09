#pragma once

#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>

namespace drl
{

	struct GameData
	{
		TerrainData terrain;
		JobData job;
		WorkerData worker;
	};

}
