#pragma once

#include <helsinki/System/glm.hpp>
#include <vector>

namespace slr
{

	struct WorldData;

	enum class SystemPlaceKind
	{
		Star,
		Jump,
		Yard
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
	constexpr float HelionStarSize = 24.0f;
	constexpr long long HelionStarPlaceId = 1;
	constexpr long long HelionKestrelJumpPlaceId = 2;
	constexpr long long HelionYardPlaceId = 3;
	const glm::vec2 HelionYardPlaceOffset{ 30.0f, 0.0f };

	float outRadiusFromStarSize(float starSize);
	float outRadiusForLayout(const SystemLayout& layout);
	glm::vec2 starPositionInSystemSpace(const SystemLayout& layout);
	const SystemPlace* starPlaceOf(const SystemLayout& layout);
	const SystemPlace* yardPlaceOf(const SystemLayout& layout);
	const SystemPlace* jumpPlaceForLaneNeighbor(const SystemLayout& layout, long long neighborSystemId);
	const SystemPlace* starPlaceForSystem(const WorldData& world, long long systemId);
	const SystemPlace* yardPlaceForSystem(const WorldData& world, long long systemId);
	const SystemPlace* jumpPlaceForLaneNeighbor(
		const WorldData& world,
		long long systemId,
		long long neighborSystemId);

	void placeJumpOnOutRadiusTowardNeighbor(
		SystemLayout& layout,
		glm::vec2 neighborGalaxyDelta,
		long long neighborSystemId);
	bool jumpPlaceIsAtOrOutsideOutRadius(const SystemLayout& layout, const SystemPlace& jumpPlace);
	void addHelionSystemPlaces(WorldData& world);

}
