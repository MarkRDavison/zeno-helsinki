#include <World/TruckCargo.hpp>
#include <algorithm>

namespace slr
{
namespace
{

	GoodStock& stockRow(StarSystem& system, long long goodId)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = goodId });
		return system.stocks.back();
	}

	StarSystem* destinationForTruck(WorldData& world, const Truck& truck)
	{
		if (truck.haulJobId != 0)
		{
			const HaulJob* job = world.haulJobById(truck.haulJobId);
			if (job != nullptr)
			{
				return world.systemById(job->destinationSystemId);
			}
		}

		if (truck.motion.route.size() < 2)
		{
			return nullptr;
		}

		return world.systemById(truck.motion.route.back());
	}

}

	void raiseInboundOnLoad(StarSystem& destination, long long goodId, long long amount)
	{
		if (amount <= 0)
		{
			return;
		}

		stockRow(destination, goodId).inbound += amount;
	}

	void reduceInboundOnArrival(StarSystem& destination, long long goodId, long long amount)
	{
		if (amount <= 0)
		{
			return;
		}

		GoodStock& stock = stockRow(destination, goodId);
		stock.inbound = std::max(0LL, stock.inbound - amount);
	}

	bool destroyTruckAndReduceInbound(WorldData& world, long long truckId)
	{
		for (auto it = world.trucks.begin(); it != world.trucks.end(); ++it)
		{
			if (it->id != truckId)
			{
				continue;
			}

			if (it->cargoAmount > 0)
			{
				StarSystem* destination = destinationForTruck(world, *it);
				if (destination != nullptr)
				{
					reduceInboundOnArrival(*destination, it->cargoGoodId, it->cargoAmount);
				}
			}

			world.trucks.erase(it);
			return true;
		}

		return false;
	}

}
