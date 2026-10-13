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

	void raiseInboundOnLoad(StarSystem& destination, long long goodId, GoodAmount amount)
	{
		if (amount <= 0.f)
		{
			return;
		}

		stockRow(destination, goodId).inbound += amount;
	}

	void reduceInboundOnArrival(StarSystem& destination, long long goodId, GoodAmount amount)
	{
		if (amount <= 0.f)
		{
			return;
		}

		GoodStock& stock = stockRow(destination, goodId);
		stock.inbound = std::max(0.f, stock.inbound - amount);
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

			const long long lostAt = truckSystemId(*it);
			if (lostAt != 0)
			{
				world.civilianLosses.push_back(CivilianLoss{
					.systemId = lostAt,
					.tick = world.tick,
					.victimEmpireId = it->empireId });
			}

			const long long convoyId = it->convoyId;
			world.trucks.erase(it);
			removeConvoyIfNoMembers(world, convoyId);
			return true;
		}

		return false;
	}

}
