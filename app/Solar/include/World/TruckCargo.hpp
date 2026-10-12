#pragma once

#include <World/WorldData.hpp>

namespace slr
{

	void raiseInboundOnLoad(StarSystem& destination, long long goodId, long long amount);
	void reduceInboundOnArrival(StarSystem& destination, long long goodId, long long amount);
	bool destroyTruckAndReduceInbound(WorldData& world, long long truckId);

}
