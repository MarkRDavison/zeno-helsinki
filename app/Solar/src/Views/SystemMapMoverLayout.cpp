#include <Views/SystemMapMoverLayout.hpp>
#include <World/LaneMotion.hpp>
#include <World/SystemPlaces.hpp>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace slr
{
namespace
{
	struct CrawlBucketKey
	{
		long long fromPlaceId{ 0 };
		long long toPlaceId{ 0 };
		int progressMilli{ 0 };

		bool operator==(const CrawlBucketKey& other) const
		{
			return fromPlaceId == other.fromPlaceId && toPlaceId == other.toPlaceId &&
				progressMilli == other.progressMilli;
		}
	};

	struct CrawlBucketKeyHash
	{
		std::size_t operator()(const CrawlBucketKey& key) const
		{
			return static_cast<std::size_t>(key.fromPlaceId) ^
				(static_cast<std::size_t>(key.toPlaceId) << 16) ^
				(static_cast<std::size_t>(key.progressMilli) << 32);
		}
	};

	void appendOrbitPositions(
		std::vector<SystemMapMoverDraw>& out,
		const std::vector<SystemMapMoverRef>& movers,
		glm::vec2 anchor,
		bool crawling)
	{
		const int total = static_cast<int>(movers.size());
		if (total <= 0)
		{
			return;
		}

		for (int slot = 0; slot < total; ++slot)
		{
			const float ringRadius = orbitRingRadiusForSlot(slot, total, crawling);
			int countOnRing = total;
			int slotOnRing = slot;
			if (!crawling && total > SystemMapOrbitMaxPerRing)
			{
				if (slot < SystemMapOrbitMaxPerRing)
				{
					countOnRing = SystemMapOrbitMaxPerRing;
					slotOnRing = slot;
				}
				else
				{
					countOnRing = total - SystemMapOrbitMaxPerRing;
					slotOnRing = slot - SystemMapOrbitMaxPerRing;
				}
			}

			SystemMapMoverDraw draw;
			draw.ref = movers[static_cast<std::size_t>(slot)];
			draw.systemPosition =
				anchor + orbitOffsetForSlot(slotOnRing, countOnRing, ringRadius);
			out.push_back(draw);
		}
	}

	void collectParkedFleetsAtPlace(
		const WorldData& world,
		long long systemId,
		long long placeId,
		std::vector<SystemMapMoverRef>& fleets)
	{
		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId != systemId || !fleetIsParked(fleet) ||
				inSystemMotionIsCrawling(fleet.inSystem) || fleet.placeId != placeId)
			{
				continue;
			}

			fleets.push_back({ SystemMapMoverKind::Fleet, fleet.id });
		}

		std::sort(
			fleets.begin(),
			fleets.end(),
			[](const SystemMapMoverRef& a, const SystemMapMoverRef& b) { return a.id < b.id; });
	}

	void collectParkedTrucksAtPlace(
		const WorldData& world,
		long long systemId,
		long long placeId,
		std::vector<SystemMapMoverRef>& movers)
	{
		for (const auto& truck : world.trucks)
		{
			if (!isTruckAtSystem(truck, systemId) || inSystemMotionIsCrawling(truck.inSystem) ||
				truck.placeId != placeId)
			{
				continue;
			}

			movers.push_back({ SystemMapMoverKind::Truck, truck.id });
		}

		std::sort(
			movers.begin(),
			movers.end(),
			[](const SystemMapMoverRef& a, const SystemMapMoverRef& b) { return a.id < b.id; });
	}
}

	glm::vec2 orbitOffsetForSlot(int slot, int countOnRing, float radius)
	{
		if (countOnRing <= 0)
		{
			return glm::vec2{ 0.0f, 0.0f };
		}

		if (countOnRing == 1)
		{
			return glm::vec2{ radius, 0.0f };
		}

		constexpr float twoPi = 6.28318530718f;
		const float angle =
			static_cast<float>(slot) * (twoPi / static_cast<float>(countOnRing));
		return glm::vec2(std::cos(angle) * radius, std::sin(angle) * radius);
	}

	float orbitRingRadiusForSlot(int slot, int totalCount, bool crawling)
	{
		if (crawling)
		{
			return SystemMapOrbitRadiusCrawl;
		}

		if (totalCount <= SystemMapOrbitMaxPerRing)
		{
			return SystemMapOrbitRadiusParked;
		}

		return slot < SystemMapOrbitMaxPerRing ? SystemMapOrbitRadiusParked : SystemMapOrbitRadiusOuter;
	}

	std::vector<SystemMapMoverDraw> listMoversForSystem(
		const WorldData& world,
		long long systemId,
		const SystemLayout& layout)
	{
		std::vector<SystemMapMoverDraw> draws;

		for (const auto& place : layout.places)
		{
			std::vector<SystemMapMoverRef> movers;
			std::vector<SystemMapMoverRef> fleets;
			std::vector<SystemMapMoverRef> trucks;
			collectParkedFleetsAtPlace(world, systemId, place.id, fleets);
			collectParkedTrucksAtPlace(world, systemId, place.id, trucks);
			movers.insert(movers.end(), fleets.begin(), fleets.end());
			movers.insert(movers.end(), trucks.begin(), trucks.end());

			appendOrbitPositions(draws, movers, place.position, false);
		}

		std::unordered_map<CrawlBucketKey, std::vector<SystemMapMoverRef>, CrawlBucketKeyHash> crawlBuckets;

		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId != systemId || !inSystemMotionIsCrawling(fleet.inSystem))
			{
				continue;
			}

			const auto anchor = inSystemMoverPosition(world, systemId, fleet.placeId, fleet.inSystem);
			if (!anchor.has_value())
			{
				continue;
			}

			CrawlBucketKey key{
				.fromPlaceId = fleet.inSystem.fromPlaceId,
				.toPlaceId = fleet.inSystem.toPlaceId,
				.progressMilli = static_cast<int>(fleet.inSystem.progress * 1000.0f),
			};
			crawlBuckets[key].push_back({ SystemMapMoverKind::Fleet, fleet.id });
		}

		for (const auto& truck : world.trucks)
		{
			if (!isTruckAtSystem(truck, systemId) || !inSystemMotionIsCrawling(truck.inSystem))
			{
				continue;
			}

			const auto anchor = inSystemMoverPosition(world, systemId, truck.placeId, truck.inSystem);
			if (!anchor.has_value())
			{
				continue;
			}

			CrawlBucketKey key{
				.fromPlaceId = truck.inSystem.fromPlaceId,
				.toPlaceId = truck.inSystem.toPlaceId,
				.progressMilli = static_cast<int>(truck.inSystem.progress * 1000.0f),
			};
			crawlBuckets[key].push_back({ SystemMapMoverKind::Truck, truck.id });
		}

		for (const auto& bucket : crawlBuckets)
		{
			std::vector<SystemMapMoverRef> movers = bucket.second;
			std::sort(
				movers.begin(),
				movers.end(),
				[](const SystemMapMoverRef& a, const SystemMapMoverRef& b)
				{
					if (a.kind != b.kind)
					{
						return a.kind == SystemMapMoverKind::Fleet;
					}

					return a.id < b.id;
				});

			const Fleet* sampleFleet = nullptr;
			for (const auto& mover : movers)
			{
				if (mover.kind == SystemMapMoverKind::Fleet)
				{
					sampleFleet = world.fleetById(mover.id);
					break;
				}
			}

			glm::vec2 anchor{ 0.0f };
			if (sampleFleet != nullptr)
			{
				const auto pos =
					inSystemMoverPosition(world, systemId, sampleFleet->placeId, sampleFleet->inSystem);
				if (pos.has_value())
				{
					anchor = *pos;
				}
			}
			else if (!movers.empty())
			{
				const Truck* truck = world.truckById(movers.front().id);
				if (truck != nullptr)
				{
					const auto pos = inSystemMoverPosition(world, systemId, truck->placeId, truck->inSystem);
					if (pos.has_value())
					{
						anchor = *pos;
					}
				}
			}

			appendOrbitPositions(draws, movers, anchor, true);
		}

		return draws;
	}

}
