#include <World/SystemPlaces.hpp>
#include <World/Goods.hpp>
#include <World/WorldData.hpp>

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

}
