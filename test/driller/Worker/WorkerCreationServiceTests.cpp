#include <catch2/catch_test_macros.hpp>
#include <Entities/Building.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Need.hpp>
#include <Entities/Worker.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace WorkerCreationServiceTests
{

	void addHousing(BuildingData& buildingData, BuildingPrototypeService& buildingPrototypes, long long beds)
	{
		BuildingPrototype proto{};
		proto.name = "Building_TestHousing";
		proto.metadata[kBuildingMetadataWorkerCapacity] = beds;
		buildingPrototypes.registerPrototype(std::move(proto));
		BuildingInstance instance{};
		instance.prototypeId = prototypeIdFromName("Building_TestHousing");
		buildingData.buildings.push_back(instance);
	}

	void addNeed(NeedPrototypeService& needs, const std::string& name)
	{
		NeedPrototype prototype{};
		prototype.name = name;
		needs.registerPrototype(std::move(prototype));
	}

	TEST_CASE("unknown prototype fails", "[drl][WorkerCreationService]")
	{
		WorkerData data;
		WorkerPrototypeService prototypes;
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		WorkerCreationService service{ data, prototypes, buildingData, buildingPrototypes, needPrototypes };
		REQUIRE_FALSE(service.createWorker(prototypeIdFromName("Worker_Builder"), glm::vec2(1.0f, 0.0f)));
		REQUIRE(data.workers.empty());
	}

	TEST_CASE("creates worker at coordinates", "[drl][WorkerCreationService]")
	{
		WorkerData data;
		WorkerPrototypeService prototypes;
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		addHousing(buildingData, buildingPrototypes, 4);
		WorkerCreationService service{ data, prototypes, buildingData, buildingPrototypes, needPrototypes };
		WorkerPrototype prototype{};
		prototype.name = "Worker_Builder";
		prototypes.registerPrototype(std::move(prototype));

		REQUIRE(service.createWorker(prototypeIdFromName("Worker_Builder"), glm::vec2(3.5f, 1.25f)));
		REQUIRE(data.workers.size() == 1);
		REQUIRE(data.workers[0].position == glm::vec2(3.5f, 1.25f));
		REQUIRE(data.workers[0].state == WorkerState::Idle);
		REQUIRE(data.workers[0].prototypeId == prototypeIdFromName("Worker_Builder"));
		REQUIRE(data.workers[0].id != 0);
	}

	TEST_CASE("create worker fills every registered need at 100", "[drl][WorkerCreationService]")
	{
		WorkerData data;
		WorkerPrototypeService prototypes;
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		addHousing(buildingData, buildingPrototypes, 4);
		addNeed(needPrototypes, "Need_Sleep");
		addNeed(needPrototypes, "Need_Food");
		addNeed(needPrototypes, "Need_Recreation");
		WorkerCreationService service{ data, prototypes, buildingData, buildingPrototypes, needPrototypes };
		WorkerPrototype prototype{};
		prototype.name = "Worker_Builder";
		prototypes.registerPrototype(std::move(prototype));

		REQUIRE(service.createWorker(prototypeIdFromName("Worker_Builder"), glm::vec2(0.0f, 0.0f)));
		REQUIRE(data.workers[0].needValues.size() == 3);
		REQUIRE(data.workers[0].needValues.at(needIdFromName("Need_Sleep")) == kNeedValueFull);
		REQUIRE(data.workers[0].needValues.at(needIdFromName("Need_Food")) == kNeedValueFull);
		REQUIRE(data.workers[0].needValues.at(needIdFromName("Need_Recreation")) == kNeedValueFull);
	}

	TEST_CASE("fifth worker with bunk of 4 fails", "[drl][WorkerCreationService]")
	{
		WorkerData data;
		WorkerPrototypeService prototypes;
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		addHousing(buildingData, buildingPrototypes, 4);
		WorkerCreationService service{ data, prototypes, buildingData, buildingPrototypes, needPrototypes };
		WorkerPrototype prototype{};
		prototype.name = "Worker_Builder";
		prototypes.registerPrototype(std::move(prototype));
		const auto id = prototypeIdFromName("Worker_Builder");
		REQUIRE(workerHousingCapacity(buildingData, buildingPrototypes) == 4);
		REQUIRE(service.createWorker(id, glm::vec2(0.0f, 0.0f)));
		REQUIRE(service.createWorker(id, glm::vec2(1.0f, 0.0f)));
		REQUIRE(service.createWorker(id, glm::vec2(2.0f, 0.0f)));
		REQUIRE(service.createWorker(id, glm::vec2(3.0f, 0.0f)));
		REQUIRE_FALSE(service.createWorker(id, glm::vec2(4.0f, 0.0f)));
		REQUIRE(data.workers.size() == 4);
	}

}
}
