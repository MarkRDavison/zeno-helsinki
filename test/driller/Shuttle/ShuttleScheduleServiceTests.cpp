#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Entities/Building.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Shuttle.hpp>
#include <Entities/Worker.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/ShuttleScheduleService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace ShuttleScheduleServiceTests
{

	struct Fixture
	{
		ShuttleData shuttleData;
		WorkerData workerData;
		BuildingData buildingData;
		WorkerPrototypeService workerPrototypes;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes, buildingData, buildingPrototypes, needPrototypes };
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttleCreation{ shuttleData, shuttlePrototypes };
		EconomyResourceService economy;
		ShuttleScheduleService schedule{
			shuttleData,
			workerData,
			recruitment,
			workerCreation,
			shuttlePrototypes,
			economy };

		Fixture()
		{
			economy.setMax(ResourceOre, -1);
			economy.set(ResourceOre, 0);
			economy.setMax(ResourceMoney, -1);
			economy.set(ResourceMoney, 500);
		}

		void registerShuttle(float idleTime, float loadingTime, float speed, bool oreCargo)
		{
			ShuttlePrototype prototype{};
			prototype.name = "Shuttle_Basic";
			prototype.idleTime = idleTime;
			prototype.loadingTime = loadingTime;
			prototype.speed = speed;
			if (oreCargo)
			{
				prototype.allowedCargo.insert(ResourceOre);
			}
			shuttlePrototypes.registerPrototype(std::move(prototype));
			REQUIRE(shuttleCreation.createShuttle(prototypeIdFromName("Shuttle_Basic")));
		}

		void addHousing(long long beds)
		{
			BuildingPrototype proto{};
			proto.name = "Building_TestHousing";
			proto.metadata[kBuildingMetadataWorkerCapacity] = beds;
			buildingPrototypes.registerPrototype(std::move(proto));
			BuildingInstance instance{};
			instance.prototypeId = prototypeIdFromName("Building_TestHousing");
			buildingData.buildings.push_back(instance);
		}

		ShuttleInstance& shuttle()
		{
			return shuttleData.shuttles[0];
		}

		void snapToSurface()
		{
			schedule.update(0.0f);
			schedule.update(1.0f);
		}
	};

	TEST_CASE("idle waits then starts travelling", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.registerShuttle(1.0f, 1.0f, 10.0f, false);
		f.schedule.update(0.9f);
		REQUIRE(f.shuttle().state == ShuttleState::Idle);
		REQUIRE_THAT(f.shuttle().elapsed, Catch::Matchers::WithinAbs(0.9f, 0.0001f));
		REQUIRE(f.shuttle().position == kShuttleStartingPosition);

		f.schedule.update(0.1f);
		REQUIRE(f.shuttle().state == ShuttleState::TravellingToSurface);
		REQUIRE(f.shuttle().elapsed == 0.0f);
		REQUIRE(f.shuttle().position == kShuttleStartingPosition);
	}

	TEST_CASE("travel partial then snap to surface", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.registerShuttle(0.0f, 1.0f, 10.0f, false);
		f.schedule.update(0.0f);
		REQUIRE(f.shuttle().state == ShuttleState::TravellingToSurface);

		f.schedule.update(1.0f);
		REQUIRE(f.shuttle().state == ShuttleState::TravellingToSurface);
		REQUIRE(f.shuttle().position != kShuttleStartingPosition);
		REQUIRE(f.shuttle().position != kShuttleSurfacePosition);

		f.schedule.update(100.0f);
		REQUIRE(f.shuttle().state == ShuttleState::WaitingOnSurface);
		REQUIRE(f.shuttle().position == kShuttleSurfacePosition);
		REQUIRE(f.shuttle().elapsed == 0.0f);
	}

	TEST_CASE("wait on surface then leave", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.registerShuttle(0.0f, 1.0f, 10000.0f, false);
		f.snapToSurface();
		REQUIRE(f.shuttle().state == ShuttleState::WaitingOnSurface);

		f.schedule.update(0.9f);
		REQUIRE(f.shuttle().state == ShuttleState::WaitingOnSurface);

		f.schedule.update(0.1f);
		REQUIRE(f.shuttle().state == ShuttleState::LeavingSurface);
		REQUIRE(f.shuttle().elapsed == 0.0f);
	}

	TEST_CASE("leave snaps then completed resets to idle at start", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.registerShuttle(0.0f, 0.0f, 10000.0f, false);
		f.snapToSurface();
		f.schedule.update(0.0f);
		REQUIRE(f.shuttle().state == ShuttleState::LeavingSurface);

		f.schedule.update(1.0f);
		REQUIRE(f.shuttle().state == ShuttleState::Completed);
		REQUIRE(f.shuttle().position == kShuttleLeavingPosition);

		f.schedule.update(0.0f);
		REQUIRE(f.shuttle().state == ShuttleState::Idle);
		REQUIRE(f.shuttle().position == kShuttleStartingPosition);
		REQUIRE(f.shuttle().elapsed == 0.0f);
	}

	TEST_CASE("moveShuttleTowardsLocation dt 0 partial and snap", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.registerShuttle(1.0f, 1.0f, 10.0f, false);
		ShuttleInstance& shuttle = f.shuttle();
		const ShuttlePrototype& prototype = f.shuttlePrototypes.getPrototype(shuttle.prototypeId);

		REQUIRE_FALSE(f.schedule.moveShuttleTowardsLocation(
			0.0f,
			shuttle,
			prototype,
			shuttle.surfacePosition,
			ShuttleState::WaitingOnSurface));
		REQUIRE(shuttle.state == ShuttleState::Idle);
		REQUIRE(shuttle.position == kShuttleStartingPosition);

		REQUIRE_FALSE(f.schedule.moveShuttleTowardsLocation(
			1.0f,
			shuttle,
			prototype,
			shuttle.surfacePosition,
			ShuttleState::WaitingOnSurface));
		REQUIRE(shuttle.state == ShuttleState::Idle);
		const float moved = glm::length(shuttle.position - kShuttleStartingPosition);
		REQUIRE_THAT(moved, Catch::Matchers::WithinAbs(10.0f, 0.0001f));

		REQUIRE(f.schedule.moveShuttleTowardsLocation(
			1000.0f,
			shuttle,
			prototype,
			shuttle.surfacePosition,
			ShuttleState::WaitingOnSurface));
		REQUIRE(shuttle.state == ShuttleState::WaitingOnSurface);
		REQUIRE(shuttle.position == kShuttleSurfacePosition);
		REQUIRE(shuttle.elapsed == 0.0f);
	}

	TEST_CASE("landing delivers demanded workers at shuttle position", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		WorkerPrototype builder{};
		builder.name = "Worker_Builder";
		f.workerPrototypes.registerPrototype(std::move(builder));
		f.addHousing(4);
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Builder", 2);
		f.registerShuttle(0.0f, 1.0f, 10000.0f, false);
		f.snapToSurface();

		REQUIRE(f.workerData.workers.size() == 2);
		REQUIRE(f.workerData.workers[0].position == kShuttleSurfacePosition);
		REQUIRE(f.workerData.workers[1].position == kShuttleSurfacePosition);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 0);
		REQUIRE_FALSE(f.schedule.consumeWorkerHousingShortage());
	}

	TEST_CASE("landing delivers only remaining housing and leaves demand", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		WorkerPrototype miner{};
		miner.name = "Worker_Miner";
		f.workerPrototypes.registerPrototype(std::move(miner));
		f.addHousing(2);
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Miner", 5);
		f.registerShuttle(0.0f, 1.0f, 10000.0f, false);
		f.snapToSurface();

		REQUIRE(f.workerData.workers.size() == 2);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 3);
		REQUIRE(f.schedule.consumeWorkerHousingShortage());
		REQUIRE_FALSE(f.schedule.consumeWorkerHousingShortage());
	}

	TEST_CASE("landing restores demand when create fails", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Miner", 2);
		f.registerShuttle(0.0f, 1.0f, 10000.0f, false);
		f.snapToSurface();

		REQUIRE(f.workerData.workers.empty());
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 2);
		REQUIRE_FALSE(f.schedule.consumeWorkerHousingShortage());
	}

	TEST_CASE("landing with no demand does not create workers", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		WorkerPrototype builder{};
		builder.name = "Worker_Builder";
		f.workerPrototypes.registerPrototype(std::move(builder));
		f.registerShuttle(0.0f, 1.0f, 10000.0f, false);
		f.snapToSurface();
		REQUIRE(f.workerData.workers.empty());
		REQUIRE_FALSE(f.schedule.consumeWorkerHousingShortage());
	}

	TEST_CASE("landing vacuums allowed ore without paying yet", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.economy.set(ResourceOre, 7);
		f.registerShuttle(0.0f, 1.0f, 10000.0f, true);
		f.snapToSurface();

		REQUIRE(f.economy.get(ResourceOre) == 0);
		REQUIRE(f.shuttle().cargo.at(ResourceOre) == 7);
		REQUIRE(f.economy.get(ResourceMoney) == 500);
		REQUIRE_FALSE(f.schedule.consumeCargoSale().has_value());
	}

	TEST_CASE("return sells cargo as money one to one", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.economy.set(ResourceOre, 7);
		f.registerShuttle(0.0f, 0.0f, 10000.0f, true);
		f.snapToSurface();
		f.schedule.update(0.0f);
		f.schedule.update(1.0f);
		REQUIRE(f.shuttle().state == ShuttleState::Completed);
		REQUIRE(f.economy.get(ResourceMoney) == 500);

		f.schedule.update(0.0f);
		REQUIRE(f.shuttle().state == ShuttleState::Idle);
		REQUIRE(f.shuttle().position == kShuttleStartingPosition);
		REQUIRE(f.shuttle().cargo.empty());
		REQUIRE(f.economy.get(ResourceMoney) == 507);
		REQUIRE(f.economy.get(ResourceOre) == 0);
		const auto sale = f.schedule.consumeCargoSale();
		REQUIRE(sale.has_value());
		REQUIRE(sale->sold.size() == 1);
		REQUIRE(sale->sold[0].first == ResourceOre);
		REQUIRE(sale->sold[0].second == 7);
		REQUIRE(sale->money == 7);
		f.economy.setHud(ResourceOre, 1, "Ore", "rock");
		f.economy.setHud(ResourceMoney, 2, "Money", "cash");
		REQUIRE(formatShuttleSaleMessage(*sale, f.economy) == "Sold 7 Ore for 7 Money");
		REQUIRE_FALSE(f.schedule.consumeCargoSale().has_value());
	}

	TEST_CASE("missing allowed cargo name is skipped", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		f.economy.set(ResourceOre, 7);
		ShuttlePrototype prototype{};
		prototype.name = "Shuttle_Basic";
		prototype.idleTime = 0.0f;
		prototype.loadingTime = 1.0f;
		prototype.speed = 10000.0f;
		prototype.allowedCargo.insert("Resource_DoesNotExist");
		f.shuttlePrototypes.registerPrototype(std::move(prototype));
		REQUIRE(f.shuttleCreation.createShuttle(prototypeIdFromName("Shuttle_Basic")));
		f.snapToSurface();

		REQUIRE(f.economy.get(ResourceOre) == 7);
		REQUIRE(f.shuttle().cargo.empty());
		REQUIRE_FALSE(f.schedule.consumeCargoSale().has_value());
	}

	TEST_CASE("leaving shuttle removes leaving workers and restores demand", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		WorkerPrototype miner{};
		miner.name = "Worker_Miner";
		f.workerPrototypes.registerPrototype(std::move(miner));
		f.addHousing(2);
		REQUIRE(f.workerCreation.createWorker(prototypeIdFromName("Worker_Miner"), glm::vec2(1.0f, 0.0f)));
		REQUIRE(f.workerCreation.createWorker(prototypeIdFromName("Worker_Miner"), glm::vec2(2.0f, 0.0f)));
		f.workerData.workers[0].leaving = true;
		f.registerShuttle(0.0f, 0.0f, 10000.0f, false);
		f.snapToSurface();
		REQUIRE(f.workerData.workers.size() == 2);

		f.schedule.update(0.0f);
		REQUIRE(f.shuttle().state == ShuttleState::LeavingSurface);
		REQUIRE(f.workerData.workers.size() == 1);
		REQUIRE_FALSE(f.workerData.workers[0].leaving);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 1);
		REQUIRE(f.workerCreation.hasSpareHousing());
		REQUIRE(f.schedule.consumeDesertedWorkerCount() == 1);
		REQUIRE(f.schedule.consumeDesertedWorkerCount() == 0);
	}

	TEST_CASE("leaving shuttle batches deserted worker count", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		WorkerPrototype miner{};
		miner.name = "Worker_Miner";
		f.workerPrototypes.registerPrototype(std::move(miner));
		f.addHousing(2);
		REQUIRE(f.workerCreation.createWorker(prototypeIdFromName("Worker_Miner"), glm::vec2(1.0f, 0.0f)));
		REQUIRE(f.workerCreation.createWorker(prototypeIdFromName("Worker_Miner"), glm::vec2(2.0f, 0.0f)));
		f.workerData.workers[0].leaving = true;
		f.workerData.workers[1].leaving = true;
		f.registerShuttle(0.0f, 0.0f, 10000.0f, false);
		f.snapToSurface();
		REQUIRE(f.schedule.consumeDesertedWorkerCount() == 0);

		f.schedule.update(0.0f);
		REQUIRE(f.workerData.workers.empty());
		REQUIRE(f.schedule.consumeDesertedWorkerCount() == 2);
		REQUIRE(f.schedule.consumeDesertedWorkerCount() == 0);
		REQUIRE(formatDesertedWorkersMessage(2) == "2 workers have left because their needs were not met.");
	}

	TEST_CASE("next landing spawns replacements for departed workers", "[drl][ShuttleScheduleService]")
	{
		Fixture f;
		WorkerPrototype miner{};
		miner.name = "Worker_Miner";
		f.workerPrototypes.registerPrototype(std::move(miner));
		f.addHousing(1);
		REQUIRE(f.workerCreation.createWorker(prototypeIdFromName("Worker_Miner"), glm::vec2(1.0f, 0.0f)));
		f.workerData.workers[0].leaving = true;
		f.registerShuttle(0.0f, 0.0f, 10000.0f, false);
		f.snapToSurface();
		f.schedule.update(0.0f);
		REQUIRE(f.workerData.workers.empty());
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 1);

		f.schedule.update(1.0f);
		REQUIRE(f.shuttle().state == ShuttleState::Completed);
		f.schedule.update(0.0f);
		REQUIRE(f.shuttle().state == ShuttleState::Idle);

		f.snapToSurface();
		REQUIRE(f.workerData.workers.size() == 1);
		REQUIRE_FALSE(f.workerData.workers[0].leaving);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 0);
	}

}
}
