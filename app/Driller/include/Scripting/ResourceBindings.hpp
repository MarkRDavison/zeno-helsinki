#pragma once

#include <Services/EconomyResourceService.hpp>
#include <sol/sol.hpp>

namespace drl
{

	void applyResourcesTable(const sol::object& resources, IEconomyResourceService& economy);

}
