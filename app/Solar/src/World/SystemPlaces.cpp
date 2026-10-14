#include <World/SystemPlaces.hpp>
#include <World/Goods.hpp>
#include <World/WorldData.hpp>
#include <algorithm>
#include <optional>

namespace slr
{
namespace
{

	constexpr float JumpRingEpsilon = 1.0e-4f;

	long long nextSystemPlaceId(const SystemLayout& layout)
	{
		long long nextId = 1;
		for (const auto& place : layout.places)
		{
			if (place.id + 1 > nextId)
			{
				nextId = place.id + 1;
			}
		}

		return nextId;
	}

	const SystemPlace* placeOfKind(const SystemLayout& layout, SystemPlaceKind kind)
	{
		for (const auto& place : layout.places)
		{
			if (place.kind == kind)
			{
				return &place;
			}
		}

		return nullptr;
	}

	void addJumpsForAllLaneNeighbors(WorldData& world, SystemLayout& layout)
	{
		const StarSystem* system = world.systemById(layout.systemId);
		if (system == nullptr)
		{
			return;
		}

		for (const auto& lane : world.hyperlanes)
		{
			long long neighborId = 0;
			if (lane.a == layout.systemId)
			{
				neighborId = lane.b;
			}
			else if (lane.b == layout.systemId)
			{
				neighborId = lane.a;
			}
			else
			{
				continue;
			}

			if (!hasHyperlane(world, layout.systemId, neighborId))
			{
				continue;
			}

			if (jumpPlaceForLaneNeighbor(layout, neighborId) != nullptr)
			{
				continue;
			}

			const StarSystem* neighbor = world.systemById(neighborId);
			if (neighbor == nullptr)
			{
				continue;
			}

			placeJumpOnOutRadiusTowardNeighbor(
				layout,
				neighbor->position - system->position,
				neighborId);
		}
	}

	SystemLayout makeLayoutWithStarAndJumps(WorldData& world, long long systemId, float starSize)
	{
		SystemLayout layout;
		layout.systemId = systemId;
		layout.starSize = starSize;
		layout.places.push_back(SystemPlace{
			.id = nextSystemPlaceId(layout),
			.kind = SystemPlaceKind::Star,
			.position = { 0.0f, 0.0f } });
		addJumpsForAllLaneNeighbors(world, layout);
		return layout;
	}

}

	float outRadiusFromStarSize(float starSize)
	{
		return OutRadiusPerStarSize * starSize;
	}

	float outRadiusForLayout(const SystemLayout& layout)
	{
		return outRadiusFromStarSize(layout.starSize);
	}

	const SystemPlace* placeById(const SystemLayout& layout, long long placeId)
	{
		for (const auto& place : layout.places)
		{
			if (place.id == placeId)
			{
				return &place;
			}
		}

		return nullptr;
	}

	const SystemPlace* pickSystemPlace(const SystemLayout& layout, glm::vec2 systemPos)
	{
		const SystemPlace* nearestMarker = nullptr;
		float nearestMarkerDistance = SystemPlaceMarkerHitRadius;
		for (const auto& place : layout.places)
		{
			if (place.kind == SystemPlaceKind::Star)
			{
				continue;
			}

			const float distance = glm::length(place.position - systemPos);
			if (distance <= SystemPlaceMarkerHitRadius && distance <= nearestMarkerDistance)
			{
				nearestMarkerDistance = distance;
				nearestMarker = &place;
			}
		}

		if (nearestMarker != nullptr)
		{
			return nearestMarker;
		}

		const SystemPlace* star = starPlaceOf(layout);
		if (star != nullptr && glm::length(star->position - systemPos) <= layout.starSize)
		{
			return star;
		}

		return nullptr;
	}

	glm::vec2 starPositionInSystemSpace(const SystemLayout& layout)
	{
		if (const SystemPlace* star = starPlaceOf(layout))
		{
			return star->position;
		}

		return glm::vec2{ 0.0f, 0.0f };
	}

	const SystemPlace* starPlaceOf(const SystemLayout& layout)
	{
		return placeOfKind(layout, SystemPlaceKind::Star);
	}

	const SystemPlace* yardPlaceOf(const SystemLayout& layout)
	{
		return placeOfKind(layout, SystemPlaceKind::Yard);
	}

	const SystemPlace* factoryPlaceOf(const SystemLayout& layout)
	{
		return placeOfKind(layout, SystemPlaceKind::Factory);
	}

	const SystemPlace* minePlaceOf(const SystemLayout& layout)
	{
		return placeOfKind(layout, SystemPlaceKind::Mine);
	}

	const SystemPlace* jumpPlaceForLaneNeighbor(const SystemLayout& layout, long long neighborSystemId)
	{
		for (const auto& place : layout.places)
		{
			if (place.kind == SystemPlaceKind::Jump && place.neighborSystemId == neighborSystemId)
			{
				return &place;
			}
		}

		return nullptr;
	}

	const SystemPlace* starPlaceForSystem(const WorldData& world, long long systemId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return nullptr;
		}

		return starPlaceOf(*layout);
	}

	const SystemPlace* yardPlaceForSystem(const WorldData& world, long long systemId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return nullptr;
		}

		return yardPlaceOf(*layout);
	}

	const SystemPlace* factoryPlaceForSystem(const WorldData& world, long long systemId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return nullptr;
		}

		return factoryPlaceOf(*layout);
	}

	const SystemPlace* minePlaceForSystem(const WorldData& world, long long systemId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return nullptr;
		}

		return minePlaceOf(*layout);
	}

	const SystemPlace* jumpPlaceForLaneNeighbor(
		const WorldData& world,
		long long systemId,
		long long neighborSystemId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return nullptr;
		}

		return jumpPlaceForLaneNeighbor(*layout, neighborSystemId);
	}

	const SystemPlace* haulLoadPlaceForGood(
		const WorldData& world,
		long long systemId,
		long long goodId)
	{
		if (world.systemLayoutBySystemId(systemId) == nullptr)
		{
			return nullptr;
		}

		for (const auto& mine : world.mines)
		{
			if (mine.systemId != systemId)
			{
				continue;
			}

			for (const auto& output : mine.outputsPerTick)
			{
				if (output.goodId == goodId)
				{
					return minePlaceForSystem(world, systemId);
				}
			}
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId == systemId && factory.outputGoodId == goodId)
			{
				return factoryPlaceForSystem(world, systemId);
			}
		}

		return yardPlaceForSystem(world, systemId);
	}

	const SystemPlace* haulDumpPlaceForGood(
		const WorldData& world,
		long long systemId,
		long long goodId)
	{
		if (world.systemLayoutBySystemId(systemId) == nullptr)
		{
			return nullptr;
		}

		for (const auto& factory : world.factories)
		{
			if (factory.systemId != systemId)
			{
				continue;
			}

			if (quantityForGood(factory.inputsPerTick, goodId) > 0.f ||
				quantityForGood(factory.desiredPiles, goodId) > 0.f)
			{
				return factoryPlaceForSystem(world, systemId);
			}
		}

		return yardPlaceForSystem(world, systemId);
	}

	const SystemPlace* parkedPlaceInSystem(
		const WorldData& world,
		long long systemId,
		long long placeId)
	{
		if (placeId == 0)
		{
			return nullptr;
		}

		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return nullptr;
		}

		return placeById(*layout, placeId);
	}

	const char* systemPlaceKindName(SystemPlaceKind kind)
	{
		switch (kind)
		{
		case SystemPlaceKind::Star:
			return "Star";
		case SystemPlaceKind::Jump:
			return "Jump";
		case SystemPlaceKind::Yard:
			return "Yard";
		case SystemPlaceKind::Factory:
			return "Factory";
		case SystemPlaceKind::Mine:
			return "Mine";
		}

		return "Place";
	}

	bool moverCanStartHyperlaneFromPlace(
		const WorldData& world,
		long long systemId,
		long long placeId,
		long long firstNeighborSystemId)
	{
		if (world.systemLayoutBySystemId(systemId) == nullptr)
		{
			return true;
		}

		const SystemPlace* outbound = jumpPlaceForLaneNeighbor(world, systemId, firstNeighborSystemId);
		return outbound != nullptr && placeId == outbound->id;
	}

	void setMoverPlaceToArrivalJump(
		const WorldData& world,
		long long arrivedSystemId,
		long long cameFromSystemId,
		long long& placeId)
	{
		const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, arrivedSystemId, cameFromSystemId);
		placeId = jump != nullptr ? jump->id : 0;
	}

	bool inSystemMotionIsCrawling(const InSystemMotion& motion)
	{
		return motion.toPlaceId != 0;
	}

	std::optional<glm::vec2> inSystemMoverPosition(
		const WorldData& world,
		long long systemId,
		long long placeId,
		const InSystemMotion& motion)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return std::nullopt;
		}

		if (inSystemMotionIsCrawling(motion))
		{
			const SystemPlace* from = placeById(*layout, motion.fromPlaceId);
			const SystemPlace* to = placeById(*layout, motion.toPlaceId);
			if (from == nullptr || to == nullptr)
			{
				return std::nullopt;
			}

			return glm::mix(from->position, to->position, motion.progress);
		}

		if (const SystemPlace* parked = placeById(*layout, placeId))
		{
			return parked->position;
		}

		return std::nullopt;
	}

	bool moversAreWithinSystemBattleMeetingRadius(
		const WorldData& world,
		long long systemId,
		long long placeIdA,
		const InSystemMotion& inSystemA,
		long long placeIdB,
		const InSystemMotion& inSystemB)
	{
		if (world.systemLayoutBySystemId(systemId) == nullptr)
		{
			return true;
		}

		const auto positionA = inSystemMoverPosition(world, systemId, placeIdA, inSystemA);
		const auto positionB = inSystemMoverPosition(world, systemId, placeIdB, inSystemB);
		if (!positionA.has_value() || !positionB.has_value())
		{
			return false;
		}

		return glm::length(*positionA - *positionB) <= SystemBattleMeetingRadius;
	}

	bool startInSystemCrawl(
		const WorldData& world,
		long long systemId,
		long long fromPlaceId,
		long long toPlaceId,
		InSystemMotion& motion,
		long long& placeId)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return false;
		}

		if (placeById(*layout, fromPlaceId) == nullptr || placeById(*layout, toPlaceId) == nullptr)
		{
			return false;
		}

		if (fromPlaceId == toPlaceId)
		{
			placeId = toPlaceId;
			motion.fromPlaceId = 0;
			motion.toPlaceId = 0;
			motion.progress = 0.0f;
			motion.pendingGalaxyRoute.clear();
			return true;
		}

		if (motion.fromPlaceId == fromPlaceId && motion.toPlaceId == toPlaceId)
		{
			return true;
		}

		motion.fromPlaceId = fromPlaceId;
		motion.toPlaceId = toPlaceId;
		motion.progress = 0.0f;
		motion.pendingGalaxyRoute.clear();
		placeId = 0;
		return true;
	}

	bool startCrawlToNeighborJump(
		const WorldData& world,
		long long systemId,
		long long neighborSystemId,
		InSystemMotion& motion,
		long long& placeId)
	{
		const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, systemId, neighborSystemId);
		if (jump == nullptr)
		{
			return false;
		}

		const long long fromPlaceId = placeId != 0 ? placeId : motion.fromPlaceId;
		return startInSystemCrawl(world, systemId, fromPlaceId, jump->id, motion, placeId);
	}

	bool advanceInSystemCrawl(
		const WorldData& world,
		long long systemId,
		InSystemMotion& motion,
		long long& placeId,
		float speed)
	{
		if (!inSystemMotionIsCrawling(motion))
		{
			return false;
		}

		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			return false;
		}

		const SystemPlace* from = placeById(*layout, motion.fromPlaceId);
		const SystemPlace* to = placeById(*layout, motion.toPlaceId);
		if (from == nullptr || to == nullptr)
		{
			return false;
		}

		const float distance = glm::length(to->position - from->position);
		if (distance <= JumpRingEpsilon || speed <= 0.0f)
		{
			placeId = motion.toPlaceId;
			motion.fromPlaceId = 0;
			motion.toPlaceId = 0;
			motion.progress = 0.0f;
			return true;
		}

		motion.progress = std::min(1.0f, motion.progress + speed / distance);
		if (motion.progress < 1.0f)
		{
			return false;
		}

		placeId = motion.toPlaceId;
		motion.fromPlaceId = 0;
		motion.toPlaceId = 0;
		motion.progress = 0.0f;
		return true;
	}

	bool beginGalaxyPathOrCrawlToOutboundJump(
		WorldData& world,
		long long systemId,
		InSystemMotion& inSystem,
		LaneMotion& motion,
		long long& placeId,
		const std::vector<long long>& path)
	{
		if (path.size() < 2)
		{
			return false;
		}

		if (inSystem.pendingGalaxyRoute == path && inSystemMotionIsCrawling(inSystem))
		{
			return true;
		}

		if (moverCanStartHyperlaneFromPlace(world, systemId, placeId, path[1]))
		{
			motion.route = path;
			motion.hop = 0;
			motion.progress = 0.0f;
			motion.direction = 1;
			placeId = 0;
			inSystem = InSystemMotion{};
			return true;
		}

		if (!startCrawlToNeighborJump(world, systemId, path[1], inSystem, placeId))
		{
			return false;
		}

		inSystem.pendingGalaxyRoute = path;
		motion.route = { systemId };
		motion.hop = 0;
		motion.progress = 0.0f;
		return true;
	}

	void applyPendingGalaxyRouteAfterCrawl(
		InSystemMotion& inSystem,
		LaneMotion& motion,
		long long&)
	{
		if (inSystemMotionIsCrawling(inSystem) || inSystem.pendingGalaxyRoute.size() < 2)
		{
			return;
		}

		motion.route = inSystem.pendingGalaxyRoute;
		motion.hop = 0;
		motion.progress = 0.0f;
		motion.direction = 1;
		inSystem = InSystemMotion{};
	}

	void continueGalaxyPathAfterHop(
		WorldData& world,
		long long& parkedSystemId,
		InSystemMotion& inSystem,
		LaneMotion& motion,
		long long& placeId,
		bool clearRouteOnLastHop)
	{
		if (motion.hop < 0 || motion.hop >= static_cast<int>(motion.route.size()))
		{
			return;
		}

		parkedSystemId = motion.route[static_cast<size_t>(motion.hop)];
		if (motion.hop >= 1)
		{
			setMoverPlaceToArrivalJump(
				world,
				parkedSystemId,
				motion.route[static_cast<size_t>(motion.hop - 1)],
				placeId);
		}

		if (motion.hop == static_cast<int>(motion.route.size()) - 1)
		{
			if (clearRouteOnLastHop)
			{
				motion.route.clear();
			}
			else
			{
				motion.route = { parkedSystemId };
			}

			motion.hop = 0;
			motion.progress = 0.0f;
			return;
		}

		const std::vector<long long> remaining(
			motion.route.begin() + motion.hop,
			motion.route.end());
		if (beginGalaxyPathOrCrawlToOutboundJump(
				world,
				parkedSystemId,
				inSystem,
				motion,
				placeId,
				remaining))
		{
			return;
		}

		motion.route = { parkedSystemId };
		motion.hop = 0;
		motion.progress = 0.0f;
		inSystem = InSystemMotion{};
	}

	void placeJumpOnOutRadiusTowardNeighbor(
		SystemLayout& layout,
		glm::vec2 neighborGalaxyDelta,
		long long neighborSystemId)
	{
		const float deltaLength = glm::length(neighborGalaxyDelta);
		if (deltaLength <= JumpRingEpsilon)
		{
			return;
		}

		const glm::vec2 unitTowardNeighbor = neighborGalaxyDelta / deltaLength;
		layout.places.push_back(SystemPlace{
			.id = nextSystemPlaceId(layout),
			.kind = SystemPlaceKind::Jump,
			.position = starPositionInSystemSpace(layout) + unitTowardNeighbor * outRadiusForLayout(layout),
			.neighborSystemId = neighborSystemId });
	}

	bool jumpPlaceIsAtOrOutsideOutRadius(const SystemLayout& layout, const SystemPlace& jumpPlace)
	{
		const float distanceFromStar = glm::length(jumpPlace.position - starPositionInSystemSpace(layout));
		return distanceFromStar + JumpRingEpsilon >= outRadiusForLayout(layout);
	}

	void addHelionSystemPlaces(WorldData& world)
	{
		SystemLayout layout = makeLayoutWithStarAndJumps(world, HelionSystemId, HelionStarSize);
		for (auto& place : layout.places)
		{
			if (place.kind == SystemPlaceKind::Star)
			{
				place.id = HelionStarPlaceId;
			}
			else if (place.kind == SystemPlaceKind::Jump && place.neighborSystemId == KestrelSystemId)
			{
				place.id = HelionKestrelJumpPlaceId;
			}
		}

		layout.places.push_back(SystemPlace{
			.id = HelionYardPlaceId,
			.kind = SystemPlaceKind::Yard,
			.position = HelionYardPlaceOffset });

		world.systemLayouts.push_back(std::move(layout));
	}

	void addJunctionSystemPlaces(WorldData& world)
	{
		SystemLayout layout = makeLayoutWithStarAndJumps(world, JunctionSystemId, HelionStarSize);
		layout.places.push_back(SystemPlace{
			.id = JunctionFactoryPlaceId,
			.kind = SystemPlaceKind::Factory,
			.position = JunctionFactoryPlaceOffset });
		world.systemLayouts.push_back(std::move(layout));
	}

	void addRimwardSystemPlaces(WorldData& world)
	{
		SystemLayout layout = makeLayoutWithStarAndJumps(world, RimwardSystemId, HelionStarSize);
		layout.places.push_back(SystemPlace{
			.id = RimwardMinePlaceId,
			.kind = SystemPlaceKind::Mine,
			.position = RimwardMinePlaceOffset });
		world.systemLayouts.push_back(std::move(layout));
	}

	void addKestrelSystemPlaces(WorldData& world)
	{
		SystemLayout layout = makeLayoutWithStarAndJumps(world, KestrelSystemId, HelionStarSize);
		layout.places.push_back(SystemPlace{
			.id = KestrelFactoryPlaceId,
			.kind = SystemPlaceKind::Factory,
			.position = KestrelFactoryPlaceOffset });
		world.systemLayouts.push_back(std::move(layout));
	}

	void addPlacesForSystemsStillMissingLayout(WorldData& world)
	{
		for (const auto& system : world.systems)
		{
			if (world.systemLayoutBySystemId(system.id) != nullptr)
			{
				continue;
			}

			SystemLayout layout = makeLayoutWithStarAndJumps(world, system.id, HelionStarSize);
			bool hasYard = false;
			bool hasMine = false;
			bool hasFactory = false;
			for (const auto& yard : world.yards)
			{
				if (yard.systemId == system.id)
				{
					hasYard = true;
					break;
				}
			}

			for (const auto& mine : world.mines)
			{
				if (mine.systemId == system.id)
				{
					hasMine = true;
					break;
				}
			}

			for (const auto& factory : world.factories)
			{
				if (factory.systemId == system.id)
				{
					hasFactory = true;
					break;
				}
			}

			if (hasYard)
			{
				layout.places.push_back(SystemPlace{
					.id = VesperYardPlaceId,
					.kind = SystemPlaceKind::Yard,
					.position = VesperYardPlaceOffset });
			}

			if (hasMine)
			{
				layout.places.push_back(SystemPlace{
					.id = VesperMinePlaceId,
					.kind = SystemPlaceKind::Mine,
					.position = VesperMinePlaceOffset });
			}

			if (hasFactory)
			{
				layout.places.push_back(SystemPlace{
					.id = AvonFactoryPlaceId,
					.kind = SystemPlaceKind::Factory,
					.position = AvonFactoryPlaceOffset });
			}

			world.systemLayouts.push_back(std::move(layout));
		}
	}

}
