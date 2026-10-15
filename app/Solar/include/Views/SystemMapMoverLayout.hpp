#pragma once

#include <World/WorldData.hpp>
#include <helsinki/System/glm.hpp>
#include <vector>

namespace slr
{

	enum class SystemMapMoverKind
	{
		Fleet,
		Truck
	};

	struct SystemMapMoverRef
	{
		SystemMapMoverKind kind{ SystemMapMoverKind::Fleet };
		long long id{ 0 };
	};

	struct SystemMapMoverDraw
	{
		SystemMapMoverRef ref;
		glm::vec2 systemPosition{ 0.0f };
	};

	constexpr int SystemMapOrbitMaxPerRing = 8;
	constexpr float SystemMapOrbitRadiusParked = 18.0f;
	constexpr float SystemMapOrbitRadiusOuter = 28.0f;
	constexpr float SystemMapOrbitRadiusCrawl = 10.0f;

	glm::vec2 orbitOffsetForSlot(int slot, int countOnRing, float radius);
	float orbitRingRadiusForSlot(int slot, int totalCount, bool crawling);

	std::vector<SystemMapMoverDraw> listMoversForSystem(
		const WorldData& world,
		long long systemId,
		const SystemLayout& layout);

}
