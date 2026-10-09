#pragma once

#include <Services/JobPrototypeService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <sol/sol.hpp>

namespace drl
{

	void bindPrototypeUserTypes(sol::state& lua);
	void applyPrototypesTable(
		const sol::object& prototypesObject,
		IJobPrototypeService& jobs,
		IWorkerPrototypeService& workers);

}
