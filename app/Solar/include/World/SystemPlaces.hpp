#pragma once

#include <helsinki/System/glm.hpp>
#include <optional>
#include <vector>

namespace slr
{

	struct WorldData;
	struct InSystemMotion;
	struct LaneMotion;

	enum class SystemPlaceKind
	{
		Star,
		Jump,
		Yard,
		Factory,
		Mine
	};

	struct SystemPlace
	{
		long long id{ 0 };
		SystemPlaceKind kind{ SystemPlaceKind::Star };
		glm::vec2 position{ 0.0f };
		long long neighborSystemId{ 0 };
	};

	struct SystemLayout
	{
		long long systemId{ 0 };
		float starSize{ 0.0f };
		std::vector<SystemPlace> places;
	};

	constexpr float OutRadiusPerStarSize = 5.0f;
	constexpr float SystemPlaceMarkerHitRadius = 10.0f;
	constexpr float HelionStarSize = 24.0f;
	constexpr long long HelionStarPlaceId = 1;
	constexpr long long HelionKestrelJumpPlaceId = 2;
	constexpr long long HelionYardPlaceId = 3;
	const glm::vec2 HelionYardPlaceOffset{ 30.0f, 0.0f };
	constexpr long long AshreachSystemId = 4;
	constexpr long long RimwardSystemId = 5;
	constexpr long long CalderaSystemId = 6;
	constexpr long long NadirSystemId = 7;
	constexpr long long VesperSystemId = 8;
	constexpr long long JunctionFactoryPlaceId = 10;
	constexpr long long RimwardMinePlaceId = 20;
	constexpr long long KestrelFactoryPlaceId = 30;
	constexpr long long AvonFactoryPlaceId = 40;
	constexpr long long VesperYardPlaceId = 50;
	constexpr long long VesperMinePlaceId = 51;
	const glm::vec2 JunctionFactoryPlaceOffset{ -28.0f, 18.0f };
	const glm::vec2 RimwardMinePlaceOffset{ 32.0f, -12.0f };
	const glm::vec2 KestrelFactoryPlaceOffset{ 18.0f, 28.0f };
	const glm::vec2 AvonFactoryPlaceOffset{ 20.0f, -24.0f };
	const glm::vec2 VesperYardPlaceOffset{ 30.0f, 0.0f };
	const glm::vec2 VesperMinePlaceOffset{ -32.0f, 12.0f };

	float outRadiusFromStarSize(float starSize);
	float outRadiusForLayout(const SystemLayout& layout);
	glm::vec2 starPositionInSystemSpace(const SystemLayout& layout);
	const SystemPlace* placeById(const SystemLayout& layout, long long placeId);
	const SystemPlace* pickSystemPlace(const SystemLayout& layout, glm::vec2 systemPos);
	const SystemPlace* starPlaceOf(const SystemLayout& layout);
	const SystemPlace* yardPlaceOf(const SystemLayout& layout);
	const SystemPlace* factoryPlaceOf(const SystemLayout& layout);
	const SystemPlace* minePlaceOf(const SystemLayout& layout);
	const SystemPlace* jumpPlaceForLaneNeighbor(const SystemLayout& layout, long long neighborSystemId);
	const SystemPlace* starPlaceForSystem(const WorldData& world, long long systemId);
	const SystemPlace* yardPlaceForSystem(const WorldData& world, long long systemId);
	const SystemPlace* factoryPlaceForSystem(const WorldData& world, long long systemId);
	const SystemPlace* minePlaceForSystem(const WorldData& world, long long systemId);
	const SystemPlace* jumpPlaceForLaneNeighbor(
		const WorldData& world,
		long long systemId,
		long long neighborSystemId);
	const SystemPlace* parkedPlaceInSystem(
		const WorldData& world,
		long long systemId,
		long long placeId);
	const char* systemPlaceKindName(SystemPlaceKind kind);
	bool moverCanStartHyperlaneFromPlace(
		const WorldData& world,
		long long systemId,
		long long placeId,
		long long firstNeighborSystemId);
	void setMoverPlaceToArrivalJump(
		const WorldData& world,
		long long arrivedSystemId,
		long long cameFromSystemId,
		long long& placeId);
	bool inSystemMotionIsCrawling(const InSystemMotion& motion);
	std::optional<glm::vec2> inSystemMoverPosition(
		const WorldData& world,
		long long systemId,
		long long placeId,
		const InSystemMotion& motion);
	bool startInSystemCrawl(
		const WorldData& world,
		long long systemId,
		long long fromPlaceId,
		long long toPlaceId,
		InSystemMotion& motion,
		long long& placeId);
	bool startCrawlToNeighborJump(
		const WorldData& world,
		long long systemId,
		long long neighborSystemId,
		InSystemMotion& motion,
		long long& placeId);
	bool advanceInSystemCrawl(
		const WorldData& world,
		long long systemId,
		InSystemMotion& motion,
		long long& placeId,
		float speed);
	bool beginGalaxyPathOrCrawlToOutboundJump(
		WorldData& world,
		long long systemId,
		InSystemMotion& inSystem,
		LaneMotion& motion,
		long long& placeId,
		const std::vector<long long>& path);
	void applyPendingGalaxyRouteAfterCrawl(
		InSystemMotion& inSystem,
		LaneMotion& motion,
		long long& placeId);

	void placeJumpOnOutRadiusTowardNeighbor(
		SystemLayout& layout,
		glm::vec2 neighborGalaxyDelta,
		long long neighborSystemId);
	bool jumpPlaceIsAtOrOutsideOutRadius(const SystemLayout& layout, const SystemPlace& jumpPlace);
	void addHelionSystemPlaces(WorldData& world);
	void addJunctionSystemPlaces(WorldData& world);
	void addRimwardSystemPlaces(WorldData& world);
	void addKestrelSystemPlaces(WorldData& world);
	void addPlacesForSystemsStillMissingLayout(WorldData& world);

}
