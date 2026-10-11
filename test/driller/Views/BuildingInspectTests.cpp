#include <catch2/catch_test_macros.hpp>
#include <Entities/Building.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Job.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Views/BuildingInspect.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace BuildingInspectTests
{

	struct Fixture
	{
		BuildingData buildings;
		JobData jobs;
		BuildingPrototypeService buildingPrototypes;
		JobPrototypeService jobPrototypes;

		Fixture()
		{
			BuildingPrototype bunk{};
			bunk.name = "Building_Bunk";
			bunk.label = "Bunk";
			bunk.size = { 2, 1 };
			bunk.metadata[kBuildingMetadataWorkerCapacity] = 4;
			buildingPrototypes.registerPrototype(std::move(bunk));

			BuildingPrototype hut{};
			hut.name = "Building_Builders_Hut";
			hut.label = "Builders Hut";
			hut.size = { 2, 1 };
			buildingPrototypes.registerPrototype(std::move(hut));

			JobPrototype sleep{};
			sleep.name = "Job_Sleep";
			jobPrototypes.registerPrototype(std::move(sleep));
		}

		BuildingInstance& addBuilding(const std::string& name, glm::ivec2 coordinates)
		{
			BuildingInstance instance = buildingPrototypes.createInstance(prototypeIdFromName(name));
			instance.coordinates = coordinates;
			buildings.buildings.push_back(instance);
			return buildings.buildings.back();
		}
	};

	TEST_CASE("pickPlacedBuildingAt misses empty tile", "[drl][BuildingInspect]")
	{
		Fixture f;
		f.addBuilding("Building_Bunk", { 1, 0 });
		REQUIRE_FALSE(pickPlacedBuildingAt(f.buildings, f.buildingPrototypes, { 3, 0 }).has_value());
	}

	TEST_CASE("pickPlacedBuildingAt hits footprint cells", "[drl][BuildingInspect]")
	{
		Fixture f;
		const BuildingInstance& bunk = f.addBuilding("Building_Bunk", { 1, 0 });
		REQUIRE(pickPlacedBuildingAt(f.buildings, f.buildingPrototypes, { 1, 0 }) == bunk.id);
		REQUIRE(pickPlacedBuildingAt(f.buildings, f.buildingPrototypes, { 2, 0 }) == bunk.id);
		REQUIRE_FALSE(pickPlacedBuildingAt(f.buildings, f.buildingPrototypes, { 1, 1 }).has_value());
	}

	TEST_CASE("snapshot missing building is empty", "[drl][BuildingInspect]")
	{
		Fixture f;
		REQUIRE_FALSE(makeBuildingInspectSnapshot(
			1,
			f.buildings,
			f.buildingPrototypes,
			f.jobs,
			f.jobPrototypes).has_value());
	}

	TEST_CASE("snapshot uses lua label, beds, and footprint jobs", "[drl][BuildingInspect]")
	{
		Fixture f;
		const BuildingInstance& bunk = f.addBuilding("Building_Bunk", { 1, 0 });
		JobInstance& sleep = f.jobs.jobs.emplace_back();
		sleep.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		sleep.tile = { 1, 0 };
		sleep.allocatedWorkerId = 7;
		JobInstance& empty = f.jobs.jobs.emplace_back();
		empty.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		empty.tile = { 2, 0 };

		const auto snapshot = makeBuildingInspectSnapshot(
			bunk.id,
			f.buildings,
			f.buildingPrototypes,
			f.jobs,
			f.jobPrototypes);
		REQUIRE(snapshot.has_value());
		REQUIRE(snapshot->label == "Bunk");
		REQUIRE(snapshot->beds == 4);
		REQUIRE(snapshot->jobs.size() == 1);
		REQUIRE(snapshot->jobs[0].label == "Sleep");
		REQUIRE(snapshot->jobs[0].inUse == 1);
		REQUIRE(snapshot->jobs[0].total == 2);
	}

	TEST_CASE("snapshot omits beds when the prototype has no capacity", "[drl][BuildingInspect]")
	{
		Fixture f;
		const BuildingInstance& hut = f.addBuilding("Building_Builders_Hut", { -2, 0 });
		const auto snapshot = makeBuildingInspectSnapshot(
			hut.id,
			f.buildings,
			f.buildingPrototypes,
			f.jobs,
			f.jobPrototypes);
		REQUIRE(snapshot.has_value());
		REQUIRE(snapshot->label == "Builders Hut");
		REQUIRE_FALSE(snapshot->beds.has_value());
		REQUIRE(snapshot->jobs.empty());
	}

}
}
