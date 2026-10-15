#pragma once

#include <Views/SystemMapCamera.hpp>
#include <Views/SystemMapMoverLayout.hpp>
#include <World/SystemPlaces.hpp>
#include <World/WorldData.hpp>
#include <imgui.h>

namespace slr
{

	enum class SystemMapPickKind
	{
		None,
		Fleet,
		Truck,
		Place
	};

	struct SystemMapPick
	{
		SystemMapPickKind kind{ SystemMapPickKind::None };
		long long id{ 0 };
	};

	SystemMapPick pickSystemMap(
		const WorldData& world,
		long long systemId,
		const SystemLayout& layout,
		const SystemMapCamera& camera,
		const std::vector<SystemMapMoverDraw>& movers,
		ImVec2 mouseScreen);

}
