#include <Scripting/NeedBindings.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <unordered_set>

namespace drl
{

	void applyNeedsTable(const sol::object& needsObject, INeedPrototypeService& needPrototypes)
	{
		if (!needsObject.is<sol::table>())
		{
			throw hl::scripting::LuaError("needs.lua did not define a needs table");
		}

		const sol::table needs = needsObject.as<sol::table>();
		std::unordered_set<int> orders;
		std::unordered_set<std::string> names;
		for (const auto& kvp : needs)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			if (!name || name->empty())
			{
				throw hl::scripting::LuaError("need entry is missing name");
			}
			if (!names.insert(*name).second)
			{
				throw hl::scripting::LuaError("duplicate need name '" + *name + "'");
			}

			sol::optional<float> decayPerSecond = row["decayPerSecond"];
			sol::optional<float> seekBelow = row["seekBelow"];
			sol::optional<int> priority = row["priority"];
			sol::optional<int> order = row["order"];
			sol::optional<std::string> label = row["label"];
			if (!decayPerSecond)
			{
				throw hl::scripting::LuaError("need '" + *name + "' is missing decayPerSecond");
			}
			if (!seekBelow)
			{
				throw hl::scripting::LuaError("need '" + *name + "' is missing seekBelow");
			}
			if (!priority)
			{
				throw hl::scripting::LuaError("need '" + *name + "' is missing priority");
			}
			if (!order)
			{
				throw hl::scripting::LuaError("need '" + *name + "' is missing order");
			}
			if (!label)
			{
				throw hl::scripting::LuaError("need '" + *name + "' is missing label");
			}
			if (!orders.insert(*order).second)
			{
				throw hl::scripting::LuaError("duplicate need order " + std::to_string(*order));
			}

			NeedPrototype prototype{};
			prototype.name = *name;
			prototype.decayPerSecond = *decayPerSecond;
			prototype.seekBelow = *seekBelow;
			prototype.priority = *priority;
			prototype.order = *order;
			prototype.label = *label;

			sol::optional<float> collapseBelow = row["collapseBelow"];
			if (collapseBelow)
			{
				prototype.collapseBelow = *collapseBelow;
			}

			needPrototypes.registerPrototype(std::move(prototype));
		}
	}

}
