#include <World/WorldData.hpp>
#include <utility>

namespace slr
{

	const Empire* WorldData::empireById(long long id) const
	{
		for (const auto& empire : empires)
		{
			if (empire.id == id)
			{
				return &empire;
			}
		}

		return nullptr;
	}

	Empire* WorldData::empireById(long long id)
	{
		return const_cast<Empire*>(static_cast<const WorldData*>(this)->empireById(id));
	}

	const StarSystem* WorldData::systemById(long long id) const
	{
		for (const auto& system : systems)
		{
			if (system.id == id)
			{
				return &system;
			}
		}

		return nullptr;
	}

	StarSystem* WorldData::systemById(long long id)
	{
		return const_cast<StarSystem*>(static_cast<const WorldData*>(this)->systemById(id));
	}

	const ShipClass* WorldData::shipClassById(long long id) const
	{
		for (const auto& shipClass : shipClasses)
		{
			if (shipClass.id == id)
			{
				return &shipClass;
			}
		}

		return nullptr;
	}

	ShipClass* WorldData::shipClassById(long long id)
	{
		return const_cast<ShipClass*>(static_cast<const WorldData*>(this)->shipClassById(id));
	}

	const Fleet* WorldData::fleetById(long long id) const
	{
		for (const auto& fleet : fleets)
		{
			if (fleet.id == id)
			{
				return &fleet;
			}
		}

		return nullptr;
	}

	Fleet* WorldData::fleetById(long long id)
	{
		return const_cast<Fleet*>(static_cast<const WorldData*>(this)->fleetById(id));
	}

	long long getStockpileAmount(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.goodAmount;
			}
		}

		return 0;
	}

	long long getInbound(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.inbound;
			}
		}

		return 0;
	}

	long long getNeed(const StarSystem& system, long long goodId)
	{
		for (const auto& stock : system.stocks)
		{
			if (stock.goodId == goodId)
			{
				return stock.need;
			}
		}

		return 0;
	}

	long long uncoveredNeed(const StarSystem& system, long long goodId)
	{
		const long long inbound = getInbound(system, goodId);
		const long long need = getNeed(system, goodId);
		if (need <= inbound)
		{
			return 0;
		}

		return need - inbound;
	}

	bool isTruckAtSystem(const Truck& truck, long long systemId)
	{
		if (truck.motion.progress != 0.0f || truck.motion.hop < 0 ||
			truck.motion.hop >= static_cast<int>(truck.motion.route.size()))
		{
			return false;
		}

		return truck.motion.route[static_cast<size_t>(truck.motion.hop)] == systemId;
	}

	long long countTrucksAtSystem(const WorldData& world, long long systemId)
	{
		long long count = 0;
		for (const auto& truck : world.trucks)
		{
			if (isTruckAtSystem(truck, systemId))
			{
				++count;
			}
		}

		return count;
	}

	long long fleetShipCount(const Fleet& fleet)
	{
		long long count = 0;
		for (const auto& row : fleet.composition)
		{
			count += row.count;
		}

		return count;
	}

	long long classCountFor(const std::vector<ClassCount>& composition, long long classId)
	{
		for (const auto& row : composition)
		{
			if (row.classId == classId)
			{
				return row.count;
			}
		}

		return 0;
	}

	bool hasHyperlane(const WorldData& world, long long a, long long b)
	{
		if (a > b)
		{
			std::swap(a, b);
		}

		for (const auto& lane : world.hyperlanes)
		{
			if (lane.a == a && lane.b == b)
			{
				return true;
			}
		}

		return false;
	}

}
