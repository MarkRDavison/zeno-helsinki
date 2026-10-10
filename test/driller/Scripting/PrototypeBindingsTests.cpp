#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Need.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/Scripting/LuaError.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <helsinki/System/glm.hpp>
#include <format>
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
		ShuttlePrototypeService shuttles;
		NeedPrototypeService needs;
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
	applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);

	const auto digId = jobPrototypeIdFromName("Job_Dig");
	REQUIRE(f.jobs.isPrototypeRegistered(digId));
	const auto& dig = f.jobs.getPrototype(digId);
	REQUIRE(dig.name == "Job_Dig");
	REQUIRE_FALSE(dig.repeats);
	REQUIRE(dig.work == 2.0f);
	REQUIRE(static_cast<bool>(dig.calculateOffset));
	REQUIRE(dig.needDecay.empty());
	REQUIRE(dig.needRestore.empty());
	REQUIRE_FALSE(dig.everyoneCanPerform);
	REQUIRE(dig.restoreUntil == kNeedValueFull);

	const auto mineJobId = jobPrototypeIdFromName("Job_Mine");
	REQUIRE(f.jobs.getPrototype(mineJobId).needDecay.empty());

	const auto builderId = prototypeIdFromName("Worker_Builder");
	REQUIRE(f.workers.isPrototypeRegistered(builderId));
	const auto& builder = f.workers.getPrototype(builderId);
	REQUIRE(builder.validJobPrototypes.contains("Job_Dig"));
	REQUIRE(builder.validJobPrototypes.contains("Job_Build_Building"));

	const auto bunkId = prototypeIdFromName("Building_Bunk");
	REQUIRE(f.buildings.isPrototypeRegistered(bunkId));
	const auto& bunk = f.buildings.getPrototype(bunkId);
	REQUIRE(bunk.label == "Bunk");
	REQUIRE(bunk.cost == 50);
	REQUIRE(bunk.size == glm::ivec2(2, 1));
	REQUIRE(bunk.texture == glm::ivec2(3, 0));
	REQUIRE(bunk.requiredWorkers.empty());
	REQUIRE(bunk.providedJobs.empty());
	REQUIRE(buildingMetadataInt(bunk, kBuildingMetadataWorkerCapacity).value() == 4);
	REQUIRE(bunk.metadata.size() == 1);

	const auto hutId = prototypeIdFromName("Building_Builders_Hut");
	REQUIRE(f.buildings.isPrototypeRegistered(hutId));
	const auto& hut = f.buildings.getPrototype(hutId);
	REQUIRE(hut.label == "Builders Hut");
	REQUIRE(hut.cost == 100);
	REQUIRE(hut.size == glm::ivec2(2, 1));
	REQUIRE(hut.texture == glm::ivec2(5, 0));
	REQUIRE(hut.requiredWorkers.at("Worker_Builder") == 2);
	REQUIRE(hut.metadata.empty());

	const auto mineId = prototypeIdFromName("Building_Mine");
	REQUIRE(f.buildings.isPrototypeRegistered(mineId));
	const auto& mine = f.buildings.getPrototype(mineId);
	REQUIRE(mine.label == "Mine");
	REQUIRE(mine.cost == 150);
	REQUIRE(mine.size == glm::ivec2(3, 1));
	REQUIRE(mine.texture == glm::ivec2(7, 0));
	REQUIRE(mine.requiredWorkers.at("Worker_Miner") == 1);
	REQUIRE(mine.providedJobs.size() == 1);
	REQUIRE(mine.providedJobs[0].first == "Job_Mine");
	REQUIRE(mine.providedJobs[0].second == glm::vec2(1.0f, 0.0f));
	REQUIRE(mine.metadata.empty());

	const auto refineId = prototypeIdFromName("Building_Refining");
	REQUIRE(f.buildings.isPrototypeRegistered(refineId));
	const auto& refine = f.buildings.getPrototype(refineId);
	REQUIRE(refine.label == "Refining");
	REQUIRE(refine.cost == 250);
	REQUIRE(refine.size == glm::ivec2(4, 1));
	REQUIRE(refine.texture == glm::ivec2(10, 0));
	REQUIRE(refine.requiredWorkers.at("Worker_Refiner") == 2);
	REQUIRE(refine.providedJobs.size() == 2);
	REQUIRE(refine.providedJobs[0].first == "Job_Refine");
	REQUIRE(refine.providedJobs[0].second == glm::vec2(0.5f, 0.0f));
	REQUIRE(refine.providedJobs[1].first == "Job_Refine");
	REQUIRE(refine.providedJobs[1].second == glm::vec2(2.5f, 0.0f));
	REQUIRE(refine.metadata.empty());

	const auto shuttleId = prototypeIdFromName("Shuttle_Basic");
	REQUIRE(f.shuttles.isPrototypeRegistered(shuttleId));
	const auto& shuttle = f.shuttles.getPrototype(shuttleId);
	REQUIRE(shuttle.size == glm::ivec2(3, 2));
	REQUIRE(shuttle.texture == glm::ivec2(0, 5));
	REQUIRE(shuttle.idleTime == 25.0f);
	REQUIRE(shuttle.loadingTime == 5.0f);
	REQUIRE(shuttle.speed == 25.0f);
	REQUIRE(shuttle.allowedCargo.contains("Resource_Ore"));
}

TEST_CASE("Lua calculateOffset is applied on a right-side tile", "[drl][Scripting]")
{
	Fixture f;
	f.lua.runFile(shipped("Scripts/Base/prototypes.lua"));
	applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);
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
	REQUIRE_THROWS_AS(applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs), hl::scripting::LuaError);
}

namespace
{

	void applyBuildingStub(Fixture& f, const std::string& buildingRow)
	{
		f.lua.runString(
			"prototypes = { jobs = {}, workers = {}, shuttles = {}, buildings = { "
			+ buildingRow
			+ " } }");
		applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);
	}

	constexpr const char* kValidBuildingFields = R"(
        name = "Building_Bunk",
        size = { x = 2, y = 1 },
        texture = { x = 3, y = 0 },
)";

}

TEST_CASE("building prototype missing label throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(
		applyBuildingStub(f, std::format("{{ {} cost = 50 }}", kValidBuildingFields)),
		hl::scripting::LuaError);
}

TEST_CASE("building prototype empty label throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(
		applyBuildingStub(f, std::format("{{ {} label = \"\", cost = 50 }}", kValidBuildingFields)),
		hl::scripting::LuaError);
}

TEST_CASE("building prototype missing cost throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(
		applyBuildingStub(f, std::format("{{ {} label = \"Bunk\" }}", kValidBuildingFields)),
		hl::scripting::LuaError);
}

TEST_CASE("building prototype negative cost throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(
		applyBuildingStub(f, std::format("{{ {} label = \"Bunk\", cost = -1 }}", kValidBuildingFields)),
		hl::scripting::LuaError);
}

TEST_CASE("building prototype negative workerCapacity throws LuaError", "[drl][Scripting]")
{
	Fixture f;
	REQUIRE_THROWS_AS(
		applyBuildingStub(
			f,
			std::format(
				"{{ {} label = \"Bunk\", cost = 50, metadata = {{ workerCapacity = -1 }} }}",
				kValidBuildingFields)),
		hl::scripting::LuaError);
}

	TEST_CASE("job needDecay parses multiplier and additive", "[drl][Scripting]")
	{
		Fixture f;
		NeedPrototype sleep{};
		sleep.name = "Need_Sleep";
		f.needs.registerPrototype(std::move(sleep));
		f.lua.runString(R"(
			prototypes = {
				jobs = {
					{
						name = "Job_Mine",
						repeats = true,
						work = 4.0,
						needDecay = {
							["Need_Sleep"] = { multiplier = 1.5, additivePerSecond = 2.0 }
						}
					}
				},
				workers = {},
				buildings = {},
				shuttles = {}
			}
		)");
		applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);
		const NeedDecayModifier& modifier =
			f.jobs.getPrototype(jobPrototypeIdFromName("Job_Mine")).needDecay.at(needIdFromName("Need_Sleep"));
		REQUIRE_THAT(modifier.multiplier, Catch::Matchers::WithinAbs(1.5f, 0.0001f));
		REQUIRE_THAT(modifier.additivePerSecond, Catch::Matchers::WithinAbs(2.0f, 0.0001f));
	}

	TEST_CASE("job needDecay omitted fields use multiplier 1 and additive 0", "[drl][Scripting]")
	{
		Fixture f;
		NeedPrototype sleep{};
		sleep.name = "Need_Sleep";
		f.needs.registerPrototype(std::move(sleep));
		f.lua.runString(R"(
			prototypes = {
				jobs = {
					{
						name = "Job_Mine",
						repeats = true,
						work = 4.0,
						needDecay = { ["Need_Sleep"] = {} }
					}
				},
				workers = {},
				buildings = {},
				shuttles = {}
			}
		)");
		applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);
		const NeedDecayModifier& modifier =
			f.jobs.getPrototype(jobPrototypeIdFromName("Job_Mine")).needDecay.at(needIdFromName("Need_Sleep"));
		REQUIRE(modifier.multiplier == 1.0f);
		REQUIRE(modifier.additivePerSecond == 0.0f);
	}

	TEST_CASE("job everyoneCanPerform and needRestore parse", "[drl][Scripting]")
	{
		Fixture f;
		NeedPrototype sleep{};
		sleep.name = "Need_Sleep";
		f.needs.registerPrototype(std::move(sleep));
		f.lua.runString(R"(
			prototypes = {
				jobs = {
					{
						name = "Job_Sleep",
						repeats = true,
						work = 1.0,
						everyoneCanPerform = true,
						needRestore = {
							["Need_Sleep"] = { restorePerSecond = 15.0 }
						}
					}
				},
				workers = {},
				buildings = {},
				shuttles = {}
			}
		)");
		applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);
		const auto& job = f.jobs.getPrototype(jobPrototypeIdFromName("Job_Sleep"));
		REQUIRE(job.everyoneCanPerform);
		REQUIRE_THAT(job.needRestore.at(needIdFromName("Need_Sleep")), Catch::Matchers::WithinAbs(15.0f, 0.0001f));
		REQUIRE(job.restoreUntil == kNeedValueFull);
	}

	TEST_CASE("job restoreUntil parses", "[drl][Scripting]")
	{
		Fixture f;
		NeedPrototype sleep{};
		sleep.name = "Need_Sleep";
		f.needs.registerPrototype(std::move(sleep));
		f.lua.runString(R"(
			prototypes = {
				jobs = {
					{
						name = "Job_Sleep",
						repeats = true,
						work = 1.0,
						restoreUntil = 50,
						needRestore = { ["Need_Sleep"] = { restorePerSecond = 15.0 } }
					}
				},
				workers = {},
				buildings = {},
				shuttles = {}
			}
		)");
		applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs);
		REQUIRE(f.jobs.getPrototype(jobPrototypeIdFromName("Job_Sleep")).restoreUntil == 50.0f);
	}

	TEST_CASE("job needRestore unknown need throws LuaError", "[drl][Scripting]")
	{
		Fixture f;
		f.lua.runString(R"(
			prototypes = {
				jobs = {
					{
						name = "Job_Sleep",
						repeats = true,
						work = 1.0,
						needRestore = { ["Need_Sleep"] = { restorePerSecond = 15.0 } }
					}
				},
				workers = {},
				buildings = {},
				shuttles = {}
			}
		)");
		REQUIRE_THROWS_AS(
			applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs),
			hl::scripting::LuaError);
	}

	TEST_CASE("job needDecay unknown need throws LuaError", "[drl][Scripting]")
	{
		Fixture f;
		f.lua.runString(R"(
			prototypes = {
				jobs = {
					{
						name = "Job_Mine",
						repeats = true,
						work = 4.0,
						needDecay = { ["Need_Sleep"] = { multiplier = 2.0 } }
					}
				},
				workers = {},
				buildings = {},
				shuttles = {}
			}
		)");
		REQUIRE_THROWS_AS(
			applyPrototypesTable(f.lua.raw()["prototypes"], f.jobs, f.workers, f.buildings, f.shuttles, f.needs),
			hl::scripting::LuaError);
	}

}
}
