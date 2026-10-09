#include <Services/BuildingPrototypeService.hpp>

namespace drl
{

	BuildingInstance BuildingPrototypeService::createInstanceFromPrototype(const BuildingPrototype& prototype)
	{
		BuildingInstance instance{};
		instance.id = allocateInstanceId();
		instance.prototypeId = prototypeIdFromName(prototype.name);
		return instance;
	}

}
