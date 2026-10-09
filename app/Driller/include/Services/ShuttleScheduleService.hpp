#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Shuttle.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{

	class IShuttleScheduleService : public IGameTickService
	{
	public:
		~IShuttleScheduleService() override = 0;

		virtual bool moveShuttleTowardsLocation(
			float delta,
			ShuttleInstance& shuttle,
			const ShuttlePrototype& prototype,
			glm::vec2 target,
			ShuttleState nextState) = 0;
	};

	inline IShuttleScheduleService::~IShuttleScheduleService() = default;

	class ShuttleScheduleService : public IShuttleScheduleService
	{
	public:
		ShuttleScheduleService(
			ShuttleData& shuttleData,
			IWorkerRecruitmentService& recruitment,
			IWorkerCreationService& workerCreation,
			const IShuttlePrototypeService& shuttlePrototypes,
			IEconomyResourceService& economy);
		~ShuttleScheduleService() override = default;

		void update(float delta) override;
		void updateShuttle(float delta, ShuttleInstance& shuttle, const ShuttlePrototype& prototype);
		bool moveShuttleTowardsLocation(
			float delta,
			ShuttleInstance& shuttle,
			const ShuttlePrototype& prototype,
			glm::vec2 target,
			ShuttleState nextState) override;

		void updateShuttleOnArrivalAtSurface(ShuttleInstance& shuttle, const ShuttlePrototype& prototype);
		void updateShuttleOnArrivalAtDepartureDestination(ShuttleInstance& shuttle);

	private:
		ShuttleData& _shuttleData;
		IWorkerRecruitmentService& _recruitment;
		IWorkerCreationService& _workerCreation;
		const IShuttlePrototypeService& _shuttlePrototypes;
		IEconomyResourceService& _economy;
	};

}
