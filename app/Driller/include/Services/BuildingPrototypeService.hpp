#pragma once

#include <Entities/Building.hpp>
#include <Services/PrototypeService.hpp>

namespace drl
{

	using IBuildingPrototypeService = IPrototypeService<BuildingInstance, BuildingPrototype>;

	class BuildingPrototypeService : public PrototypeService<BuildingInstance, BuildingPrototype>
	{
	protected:
		BuildingInstance createInstanceFromPrototype(const BuildingPrototype& prototype) override;
	};

}
