#include <Scripting/ResourceBindings.hpp>
#include <helsinki/Scripting/LuaError.hpp>

namespace drl
{

	void applyResourcesTable(const sol::object& resourcesObject, IEconomyResourceService& economy)
	{
		if (!resourcesObject.is<sol::table>())
		{
			throw hl::scripting::LuaError("resources.lua did not define a resources table");
		}

		const sol::table resources = resourcesObject.as<sol::table>();
		for (const auto& kvp : resources)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			if (!name || name->empty())
			{
				throw hl::scripting::LuaError("resource entry is missing name");
			}

			sol::optional<long long> max = row["max"];
			sol::optional<long long> amount = row["amount"];
			if (!max || !amount)
			{
				throw hl::scripting::LuaError("resource '" + *name + "' is missing max or amount");
			}

			economy.setMax(*name, *max);
			economy.set(*name, *amount);
		}
	}

}
