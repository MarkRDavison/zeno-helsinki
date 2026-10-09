#pragma once

#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <sol/sol.hpp>

namespace drl
{

	void bindPrototypeUserTypes(sol::state& lua);
	void applyPrototypesTable(
		const sol::object& prototypesObject,
		IJobPrototypeService& jobs,
		IWorkerPrototypeService& workers,
		IBuildingPrototypeService& buildings,
		IShuttlePrototypeService& shuttles);

}
