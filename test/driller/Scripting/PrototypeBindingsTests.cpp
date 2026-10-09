#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <helsinki/System/glm.hpp>
#include <string>

namespace drl
{
namespace PrototypeBindingsTests
{

struct Fixture
	{
		hl::scripting::LuaState lua;
		TerrainData terrainData;
		JobData jobData;
		TerrainAlterationService terrain{ terrainData };
		JobPrototypeService jobs;
		WorkerPrototypeService workers;
		BuildingPrototypeService buildings;
		JobCreationService jobCreation{ jobData, jobs, terrain };

		Fixture()
		{
			bindPrototypeUserTypes(lua.raw());
		}
	};

std::string shipped(const char* relative)
{
	return std::string(DRILLER_DATA_DIR) + "/" + relative;
}

TEST_CASE("shipped prototypes.lua registers Job_Dig and Worker_Builder", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runFile(shipped("Scripts/Base/prototypes.lua"));
	applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings);

	const auto digId = jobPrototypeIdFromName("Job_Dig");
	REQUIRE(f.jobs.isPrototypeRegistered(digId));
	const auto& dig = f.jobs.getPrototype(digId);
	REQUIRE(dig.name == "Job_Dig");
	REQUIRE_FALSE(dig.repeats);
	REQUIRE(dig.work == 2.0f);
	REQUIRE(static_cast<bool>(dig.calculateOffset));

	const auto builderId = prototypeIdFromName("Worker_Builder");
	REQUIRE(f.workers.isPrototypeRegistered(builderId));
	const auto& builder = f.workers.getPrototype(builderId);
	REQUIRE(builder.validJobPrototypes.contains("Job_Dig"));
	REQUIRE(builder.validJobPrototypes.contains("Job_Build_Building"));

	const auto bunkId = prototypeIdFromName("Building_Bunk");
	REQUIRE(f.buildings.isPrototypeRegistered(bunkId));
	const auto& bunk = f.buildings.getPrototype(bunkId);
	REQUIRE(bunk.size == glm::ivec2(2, 1));
	REQUIRE(bunk.texture == glm::ivec2(3, 0));
	REQUIRE(bunk.requiredWorkers.empty());
	REQUIRE(bunk.providedJobs.empty());

	const auto hutId = prototypeIdFromName("Building_Builders_Hut");
	REQUIRE(f.buildings.isPrototypeRegistered(hutId));
	const auto& hut = f.buildings.getPrototype(hutId);
	REQUIRE(hut.size == glm::ivec2(2, 1));
	REQUIRE(hut.texture == glm::ivec2(5, 0));
	REQUIRE(hut.requiredWorkers.at("Worker_Builder") == 2);

	const auto mineId = prototypeIdFromName("Building_Mine");
	REQUIRE(f.buildings.isPrototypeRegistered(mineId));
	const auto& mine = f.buildings.getPrototype(mineId);
	REQUIRE(mine.size == glm::ivec2(3, 1));
	REQUIRE(mine.texture == glm::ivec2(7, 0));
	REQUIRE(mine.requiredWorkers.at("Worker_Miner") == 1);
	REQUIRE(mine.providedJobs.size() == 1);
	REQUIRE(mine.providedJobs[0].first == "Job_Mine");
	REQUIRE(mine.providedJobs[0].second == glm::vec2(1.0f, 0.0f));

	const auto refineId = prototypeIdFromName("Building_Refining");
	REQUIRE(f.buildings.isPrototypeRegistered(refineId));
	const auto& refine = f.buildings.getPrototype(refineId);
	REQUIRE(refine.size == glm::ivec2(4, 1));
	REQUIRE(refine.texture == glm::ivec2(10, 0));
	REQUIRE(refine.requiredWorkers.at("Worker_Refiner") == 2);
	REQUIRE(refine.providedJobs.size() == 2);
	REQUIRE(refine.providedJobs[0].first == "Job_Refine");
	REQUIRE(refine.providedJobs[0].second == glm::vec2(0.5f, 0.0f));
	REQUIRE(refine.providedJobs[1].first == "Job_Refine");
	REQUIRE(refine.providedJobs[1].second == glm::vec2(2.5f, 0.0f));
}

TEST_CASE("Lua calculateOffset is applied on a right-side tile", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runFile(shipped("Scripts/Base/prototypes.lua"));
	applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings);
	REQUIRE(f.terrain.digShaft(0));
	f.terrain.initialiseTile(0, 1);
	REQUIRE(f.jobCreation.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(1, 0)));
	REQUIRE_THAT(f.jobData.jobs[0].offset.x, Catch::Matchers::WithinAbs(-0.5f, 0.0001f));
	REQUIRE_THAT(f.jobData.jobs[0].offset.y, Catch::Matchers::WithinAbs(0.0f, 0.0001f));
}

TEST_CASE("missing prototypes table throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runString("x = 1");
	REQUIRE_THROWS_AS(applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings), hl::scripting::LuaError);
}

}
}
