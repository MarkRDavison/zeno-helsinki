#include <Scripting/ResourceBindings.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <unordered_set>

namespace drl
{

	void applyResourcesTable(const sol::object& resourcesObject, IEconomyResourceService& economy)
	{
		if (!resourcesObject.is<sol::table>())
		{
			throw hl::scripting::LuaError("resources.lua did not define a resources table");
		}

		const sol::table resources = resourcesObject.as<sol::table>();
		std::unordered_set<int> orders;
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
			sol::optional<int> order = row["order"];
			sol::optional<std::string> label = row["label"];
			sol::optional<std::string> description = row["description"];
			if (!max || !amount)
			{
				throw hl::scripting::LuaError("resource '" + *name + "' is missing max or amount");
			}
			if (!order)
			{
				throw hl::scripting::LuaError("resource '" + *name + "' is missing order");
			}
			if (!label)
			{
				throw hl::scripting::LuaError("resource '" + *name + "' is missing label");
			}
			if (!description)
			{
				throw hl::scripting::LuaError("resource '" + *name + "' is missing description");
			}
			if (!orders.insert(*order).second)
			{
				throw hl::scripting::LuaError("duplicate resource order " + std::to_string(*order));
			}

			economy.setMax(*name, *max);
			economy.set(*name, *amount);
			economy.setHud(*name, *order, *label, *description);
		}
	}

}
