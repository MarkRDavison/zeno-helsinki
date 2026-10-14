#pragma once

#include <World/StepGalaxy.hpp>
#include <World/SystemPlaces.hpp>
#include <World/WorldData.hpp>

namespace slr
{

	inline long long parkedInstallationPlaceId(const WorldData& world, long long systemId)
	{
		if (const SystemPlace* mine = minePlaceForSystem(world, systemId))
		{
			return mine->id;
		}

		if (const SystemPlace* yard = yardPlaceForSystem(world, systemId))
		{
			return yard->id;
		}

		if (const SystemPlace* factory = factoryPlaceForSystem(world, systemId))
		{
			return factory->id;
		}

		return 0;
	}

	inline void putFleetAtJumpToward(WorldData& world, long long fleetId, long long neighborSystemId)
	{
		Fleet* fleet = world.fleetById(fleetId);
		if (fleet == nullptr)
		{
			return;
		}

		const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, fleet->systemId, neighborSystemId);
		if (jump != nullptr)
		{
			fleet->placeId = jump->id;
		}
	}

	inline void putTrucksAtSystemJumpToward(
		WorldData& world,
		long long systemId,
		long long neighborSystemId)
	{
		const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, systemId, neighborSystemId);
		if (jump == nullptr)
		{
			return;
		}

		for (auto& truck : world.trucks)
		{
			if (isTruckAtSystem(truck, systemId))
			{
				truck.placeId = jump->id;
			}
		}
	}

	inline void stepUntilTruckHasGalaxyRoute(WorldData& world, long long truckId, int maxTicks = 256)
	{
		for (int i = 0; i < maxTicks; ++i)
		{
			const Truck* truck = world.truckById(truckId);
			if (truck != nullptr && truck->motion.route.size() >= 2)
			{
				return;
			}

			stepGalaxy(world);
		}
	}

	inline void stepUntilConvoyUnderway(WorldData& world, long long convoyId, int maxTicks = 256)
	{
		for (int i = 0; i < maxTicks; ++i)
		{
			const Convoy* convoy = world.convoyById(convoyId);
			if (convoy != nullptr && convoyIsUnderway(*convoy))
			{
				return;
			}

			stepGalaxy(world);
		}
	}

	inline void stepUntilFleetHasGalaxyRoute(WorldData& world, long long fleetId, int maxTicks = 256)
	{
		for (int i = 0; i < maxTicks; ++i)
		{
			const Fleet* fleet = world.fleetById(fleetId);
			if (fleet != nullptr && fleet->motion.route.size() >= 2)
			{
				return;
			}

			stepGalaxy(world);
		}
	}

	inline void stepUntilTruckAtPlace(WorldData& world, long long truckId, long long placeId, int maxTicks = 256)
	{
		for (int i = 0; i < maxTicks; ++i)
		{
			const Truck* truck = world.truckById(truckId);
			if (truck != nullptr && truck->placeId == placeId &&
				!inSystemMotionIsCrawling(truck->inSystem))
			{
				return;
			}

			stepGalaxy(world);
		}
	}

	inline void stepUntilStockpileAtLeast(
		WorldData& world,
		long long systemId,
		long long goodId,
		GoodAmount amount,
		int maxTicks = 256)
	{
		for (int i = 0; i < maxTicks; ++i)
		{
			const StarSystem* system = world.systemById(systemId);
			if (system != nullptr && getStockpileAmount(*system, goodId) >= amount)
			{
				return;
			}

			stepGalaxy(world);
		}
	}

}
