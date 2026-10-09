#pragma once

#include <Entities/Data/ShuttleData.hpp>
#include <Services/ShuttlePrototypeService.hpp>

namespace drl
{

	class IShuttleCreationService
	{
	public:
		virtual ~IShuttleCreationService() = 0;

		virtual bool createShuttle(ShuttlePrototypeId prototypeId) = 0;
	};

	inline IShuttleCreationService::~IShuttleCreationService() = default;

	class ShuttleCreationService : public IShuttleCreationService
	{
	public:
		ShuttleCreationService(
			ShuttleData& shuttleData,
			IShuttlePrototypeService& shuttlePrototypeService);
		~ShuttleCreationService() override = default;

		bool createShuttle(ShuttlePrototypeId prototypeId) override;

	private:
		ShuttleData& _shuttleData;
		IShuttlePrototypeService& _shuttlePrototypeService;
	};

}
