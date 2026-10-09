#pragma once

#include <Entities/Building.hpp>
#include <Services/PrototypeService.hpp>
#include <string>
#include <vector>

namespace drl
{

	using IBuildingPrototypeService = IPrototypeService<BuildingInstance, BuildingPrototype>;

	class BuildingPrototypeService : public PrototypeService<BuildingInstance, BuildingPrototype>
	{
	public:
		void registerPrototype(BuildingPrototype prototype) override;
		const std::vector<std::string>& registeredNames() const { return _registeredNames; }

	protected:
		BuildingInstance createInstanceFromPrototype(const BuildingPrototype& prototype) override;

	private:
		std::vector<std::string> _registeredNames;
	};

}
