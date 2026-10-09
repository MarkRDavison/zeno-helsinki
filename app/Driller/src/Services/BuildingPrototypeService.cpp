#include <Services/BuildingPrototypeService.hpp>

namespace drl
{

	void BuildingPrototypeService::registerPrototype(BuildingPrototype prototype)
	{
		_registeredNames.push_back(prototype.name);
		PrototypeService::registerPrototype(std::move(prototype));
	}

	BuildingInstance BuildingPrototypeService::createInstanceFromPrototype(const BuildingPrototype& prototype)
	{
		BuildingInstance instance{};
		instance.id = allocateInstanceId();
		instance.prototypeId = prototypeIdFromName(prototype.name);
		return instance;
	}

}
