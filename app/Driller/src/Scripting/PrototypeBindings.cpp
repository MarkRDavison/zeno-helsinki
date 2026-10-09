#include <Scripting/PrototypeBindings.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/System/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_set>

namespace drl
{

	void bindPrototypeUserTypes(sol::state& lua)
	{
		lua.new_usertype<glm::vec2>(
			"vec2f",
			sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
			"x", &glm::vec2::x,
			"y", &glm::vec2::y);

		lua.new_usertype<glm::ivec2>(
			"vec2i",
			sol::constructors<glm::ivec2(), glm::ivec2(int, int)>(),
			"x", &glm::ivec2::x,
			"y", &glm::ivec2::y);

		lua.new_usertype<JobInstance>(
			"JobInstance",
			"id", &JobInstance::id,
			"prototypeId", &JobInstance::prototypeId,
			"additionalPrototypeId", &JobInstance::additionalPrototypeId,
			"allocatedWorkerId", &JobInstance::allocatedWorkerId,
			"tile", &JobInstance::tile,
			"offset", &JobInstance::offset,
			"requiresRemoval", &JobInstance::requiresRemoval,
			"work", &JobInstance::work);
	}

	namespace
	{
		sol::table requireTable(const sol::object& object, const char* what)
		{
			if (!object.is<sol::table>())
			{
				throw hl::scripting::LuaError(std::string(what) + " is missing");
			}
			return object.as<sol::table>();
		}

		std::function<glm::vec2(const JobInstance&, const JobPrototype&)> wrapCalculateOffset(
			const sol::object& offsetObject)
		{
			if (!offsetObject.valid() || offsetObject.get_type() != sol::type::function)
			{
				return {};
			}

			sol::protected_function fn = offsetObject;
			return [fn](const JobInstance& job, const JobPrototype&)
			{
				sol::protected_function_result result = fn(job);
				if (!result.valid())
				{
					const sol::error err = result;
					std::cerr << "[hl::scripting] " << err.what() << '\n';
					return glm::vec2(0.0f, 0.0f);
				}

				if (result.get_type() == sol::type::nil)
				{
					return glm::vec2(0.0f, 0.0f);
				}

				return result.get<glm::vec2>();
			};
		}
	}

	void applyPrototypesTable(
		const sol::object& prototypesObject,
		IJobPrototypeService& jobs,
		IWorkerPrototypeService& workers)
	{
		const sol::table prototypes = requireTable(prototypesObject, "prototypes");
		const sol::table jobRows = requireTable(prototypes["jobs"], "prototypes.jobs");
		const sol::table workerRows = requireTable(prototypes["workers"], "prototypes.workers");

		for (const auto& kvp : jobRows)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			sol::optional<bool> repeats = row["repeats"];
			sol::optional<float> work = row["work"];
			if (!name || name->empty() || !repeats || !work)
			{
				throw hl::scripting::LuaError("job prototype is missing name, repeats, or work");
			}

			JobPrototype prototype{};
			prototype.name = *name;
			prototype.repeats = *repeats;
			prototype.work = *work;
			if (row["onComplete"].get_type() == sol::type::function)
			{
				prototype.onComplete = row["onComplete"];
			}
			prototype.calculateOffset = wrapCalculateOffset(row["calculateOffset"]);
			jobs.registerPrototype(std::move(prototype));
		}

		for (const auto& kvp : workerRows)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			if (!name || name->empty())
			{
				throw hl::scripting::LuaError("worker prototype is missing name");
			}

			WorkerPrototype prototype{};
			prototype.name = *name;
			const sol::object jobsObject = row["jobs"];
			if (jobsObject.is<sol::table>())
			{
				const sol::table jobNames = jobsObject.as<sol::table>();
				for (const auto& jobKvp : jobNames)
				{
					if (!jobKvp.second.is<std::string>())
					{
						continue;
					}
					std::string jobName = jobKvp.second.as<std::string>();
					if (!jobName.empty())
					{
						prototype.validJobPrototypes.insert(std::move(jobName));
					}
				}
			}

			workers.registerPrototype(std::move(prototype));
		}
	}

}
