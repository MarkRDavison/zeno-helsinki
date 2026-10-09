#pragma once

#include <Services/UpgradeService.hpp>
#include <sol/sol.hpp>

namespace drl
{

	void applyUpgradesTable(const sol::object& upgrades, IUpgradeService& upgradeService);

}
