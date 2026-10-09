#pragma once

#include <Entities/Shuttle.hpp>
#include <Services/PrototypeService.hpp>

namespace drl
{

	using IShuttlePrototypeService = IPrototypeService<ShuttleInstance, ShuttlePrototype>;

	class ShuttlePrototypeService : public PrototypeService<ShuttleInstance, ShuttlePrototype>
	{
	protected:
		ShuttleInstance createInstanceFromPrototype(const ShuttlePrototype& prototype) override;
	};

}
