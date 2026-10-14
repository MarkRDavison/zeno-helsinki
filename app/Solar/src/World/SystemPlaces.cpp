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
		SystemLayout layout;
		layout.systemId = HelionSystemId;
		layout.starSize = HelionStarSize;
		layout.places.push_back(SystemPlace{
			.id = HelionStarPlaceId,
			.kind = SystemPlaceKind::Star,
			.position = { 0.0f, 0.0f } });

		const StarSystem* helion = world.systemById(HelionSystemId);
		const StarSystem* kestrel = world.systemById(KestrelSystemId);
		if (helion != nullptr && kestrel != nullptr && hasHyperlane(world, HelionSystemId, KestrelSystemId))
		{
			placeJumpOnOutRadiusTowardNeighbor(
				layout,
				kestrel->position - helion->position,
				KestrelSystemId);
			if (layout.places.back().kind == SystemPlaceKind::Jump)
			{
				layout.places.back().id = HelionKestrelJumpPlaceId;
			}
		}

		layout.places.push_back(SystemPlace{
			.id = HelionYardPlaceId,
			.kind = SystemPlaceKind::Yard,
			.position = HelionYardPlaceOffset });

		world.systemLayouts.push_back(std::move(layout));
	}

}
