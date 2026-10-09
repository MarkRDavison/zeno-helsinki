#pragma once

#include <Services/GameCommandService.hpp>
#include <sol/sol.hpp>

namespace drl
{

	void bindGameCommands(sol::state& lua, IGameCommandService& commands);

}
