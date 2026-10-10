#pragma once

#include <Services/NeedPrototypeService.hpp>
#include <sol/sol.hpp>

namespace drl
{

	void applyNeedsTable(const sol::object& needs, INeedPrototypeService& needPrototypes);

}
