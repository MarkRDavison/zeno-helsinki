#pragma once

#include <World/WorldData.hpp>

namespace slr
{

	void raiseInboundOnLoad(StarSystem& destination, long long goodId, GoodAmount amount);
	void reduceInboundOnArrival(StarSystem& destination, long long goodId, GoodAmount amount);
	bool destroyTruckAndReduceInbound(WorldData& world, long long truckId);

}
