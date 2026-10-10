#include <Scripting/PrototypeBindings.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/System/glm.hpp>
#include <cmath>
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

		void logProtectedError(sol::protected_function_result& result)
		{
			const sol::error err = result;
			std::cerr << "[hl::scripting] " << err.what() << '\n';
		}

		std::function<void(const JobInstance&)> wrapOnComplete(const sol::object& completeObject)
		{
			if (!completeObject.valid() || completeObject.get_type() != sol::type::function)
			{
				return {};
			}

			sol::protected_function fn = completeObject;
			return [fn](const JobInstance& job)
			{
				sol::protected_function_result result = fn(job);
				if (!result.valid())
				{
					logProtectedError(result);
				}
			};
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
					logProtectedError(result);
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
		IWorkerPrototypeService& workers,
		IBuildingPrototypeService& buildings,
		IShuttlePrototypeService& shuttles)
	{
		const sol::table prototypes = requireTable(prototypesObject, "prototypes");
		const sol::table jobRows = requireTable(prototypes["jobs"], "prototypes.jobs");
		const sol::table workerRows = requireTable(prototypes["workers"], "prototypes.workers");
		const sol::table buildingRows = requireTable(prototypes["buildings"], "prototypes.buildings");
		const sol::table shuttleRows = requireTable(prototypes["shuttles"], "prototypes.shuttles");

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
			prototype.onComplete = wrapOnComplete(row["onComplete"]);
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

		for (const auto& kvp : buildingRows)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			const sol::table size = requireTable(row["size"], "building size");
			const sol::table texture = requireTable(row["texture"], "building texture");
			sol::optional<int> sizeX = size["x"];
			sol::optional<int> sizeY = size["y"];
			sol::optional<int> textureX = texture["x"];
			sol::optional<int> textureY = texture["y"];
			sol::optional<std::string> label = row["label"];
			sol::optional<long long> cost = row["cost"];
			if (!name || name->empty() || !sizeX || !sizeY || !textureX || !textureY)
			{
				throw hl::scripting::LuaError("building prototype is missing name, size, or texture");
			}

			if (!label || label->empty() || !cost || *cost < 0)
			{
				throw hl::scripting::LuaError("building prototype is missing label or cost");
			}

			BuildingPrototype prototype{};
			prototype.name = *name;
			prototype.label = *label;
			prototype.cost = *cost;
			prototype.size = glm::ivec2(*sizeX, *sizeY);
			prototype.texture = glm::ivec2(*textureX, *textureY);

			const sol::object metadataObject = row["metadata"];
			if (metadataObject.valid() && metadataObject.get_type() != sol::type::nil)
			{
				if (!metadataObject.is<sol::table>())
				{
					throw hl::scripting::LuaError("building metadata must be a table of integer values");
				}

				const sol::table metadataRows = metadataObject.as<sol::table>();
				for (const auto& metadataKvp : metadataRows)
				{
					if (metadataKvp.first.get_type() != sol::type::string)
					{
						throw hl::scripting::LuaError("building metadata keys must be strings");
					}

					if (metadataKvp.second.get_type() != sol::type::number)
					{
						throw hl::scripting::LuaError("building metadata values must be integers");
					}

					const double raw = metadataKvp.second.as<double>();
					if (raw != std::floor(raw))
					{
						throw hl::scripting::LuaError("building metadata values must be integers");
					}

					const std::string key = metadataKvp.first.as<std::string>();
					const long long value = static_cast<long long>(raw);
					if (key == kBuildingMetadataWorkerCapacity && value < 0)
					{
						throw hl::scripting::LuaError("building workerCapacity cannot be negative");
					}

					prototype.metadata[key] = value;
				}
			}

			const sol::object workersObject = row["workers"];
			if (workersObject.is<sol::table>())
			{
				const sol::table workerRowsForBuilding = workersObject.as<sol::table>();
				for (const auto& workerKvp : workerRowsForBuilding)
				{
					if (!workerKvp.second.is<sol::table>())
					{
						continue;
					}

					const sol::table workerRow = workerKvp.second.as<sol::table>();
					sol::optional<std::string> workerName = workerRow["name"];
					sol::optional<int> amount = workerRow["amount"];
					if (!workerName || workerName->empty() || !amount)
					{
						throw hl::scripting::LuaError("building worker entry is missing name or amount");
					}
					prototype.requiredWorkers[*workerName] = *amount;
				}
			}

			const sol::object jobsObjectForBuilding = row["jobs"];
			if (jobsObjectForBuilding.is<sol::table>())
			{
				const sol::table jobRowsForBuilding = jobsObjectForBuilding.as<sol::table>();
				for (const auto& jobKvp : jobRowsForBuilding)
				{
					if (!jobKvp.second.is<sol::table>())
					{
						continue;
					}

					const sol::table jobRow = jobKvp.second.as<sol::table>();
					sol::optional<std::string> jobName = jobRow["name"];
					const sol::table offset = requireTable(jobRow["offset"], "building job offset");
					sol::optional<float> offsetX = offset["x"];
					sol::optional<float> offsetY = offset["y"];
					if (!jobName || jobName->empty() || !offsetX || !offsetY)
					{
						throw hl::scripting::LuaError("building job entry is missing name or offset");
					}
					prototype.providedJobs.emplace_back(*jobName, glm::vec2(*offsetX, *offsetY));
				}
			}

			buildings.registerPrototype(std::move(prototype));
		}

		for (const auto& kvp : shuttleRows)
		{
			if (!kvp.second.is<sol::table>())
			{
				continue;
			}

			const sol::table row = kvp.second.as<sol::table>();
			sol::optional<std::string> name = row["name"];
			const sol::table size = requireTable(row["size"], "shuttle size");
			const sol::table texture = requireTable(row["texture"], "shuttle texture");
			sol::optional<int> sizeX = size["x"];
			sol::optional<int> sizeY = size["y"];
			sol::optional<int> textureX = texture["x"];
			sol::optional<int> textureY = texture["y"];
			sol::optional<float> idleTime = row["idleTime"];
			sol::optional<float> loadingTime = row["loadingTime"];
			sol::optional<float> speed = row["speed"];
			if (!name || name->empty() || !sizeX || !sizeY || !textureX || !textureY ||
				!idleTime || !loadingTime || !speed)
			{
				throw hl::scripting::LuaError(
					"shuttle prototype is missing name, size, texture, idleTime, loadingTime, or speed");
			}

			ShuttlePrototype prototype{};
			prototype.name = *name;
			prototype.size = glm::ivec2(*sizeX, *sizeY);
			prototype.texture = glm::ivec2(*textureX, *textureY);
			prototype.idleTime = *idleTime;
			prototype.loadingTime = *loadingTime;
			prototype.speed = *speed;

			const sol::object cargoObject = row["allowedCargo"];
			if (cargoObject.is<sol::table>())
			{
				const sol::table cargoRows = cargoObject.as<sol::table>();
				for (const auto& cargoKvp : cargoRows)
				{
					if (!cargoKvp.second.is<std::string>())
					{
						continue;
					}
					std::string cargoName = cargoKvp.second.as<std::string>();
					if (!cargoName.empty())
					{
						prototype.allowedCargo.insert(std::move(cargoName));
					}
				}
			}

			shuttles.registerPrototype(std::move(prototype));
		}
	}

}
