#include <Scripting/UpgradeBindings.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <unordered_set>

namespace drl
{

	void applyUpgradesTable(const sol::object& upgradesObject, IUpgradeService& upgradeService)
	{
		if (!upgradesObject.is<sol::table>())
		{
			throw hl::scripting::LuaError("upgrades.lua did not define an upgrades table");
		}

		const sol::table upgrades = upgradesObject.as<sol::table>();
		std::unordered_set<int> orders;
		for (const auto& kvp : upgrades)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			if (!name || name->empty())
			{
				throw hl::scripting::LuaError("upgrade entry is missing name");
			}

			sol::optional<int> order = row["order"];
			sol::optional<std::string> label = row["label"];
			sol::optional<std::string> description = row["description"];
			if (!order)
			{
				throw hl::scripting::LuaError("upgrade '" + *name + "' is missing order");
			}
			if (!label)
			{
				throw hl::scripting::LuaError("upgrade '" + *name + "' is missing label");
			}
			if (!description)
			{
				throw hl::scripting::LuaError("upgrade '" + *name + "' is missing description");
			}
			if (!orders.insert(*order).second)
			{
				throw hl::scripting::LuaError("duplicate upgrade order " + std::to_string(*order));
			}

			upgradeService.registerHud(*name, *order, *label, *description);
		}
	}

}
