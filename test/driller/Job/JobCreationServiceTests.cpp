#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <helsinki/System/glm.hpp>

using drl::JobCreationService;
using drl::JobData;
using drl::JobPrototype;
using drl::JobPrototypeService;
using drl::TerrainAlterationService;
using drl::TerrainData;
using drl::jobPrototypeIdFromName;

namespace
{
	struct Fixture
	{
		TerrainData terrainData;
		JobData jobData;
		TerrainAlterationService terrain{ terrainData };
		JobPrototypeService prototypes;
		JobCreationService jobs{ jobData, prototypes, terrain };

		void registerDigJob(float work = 1.5f)
		{
			JobPrototype prototype{};
			prototype.name = "Job_Dig";
			prototype.work = work;
			prototypes.registerPrototype(std::move(prototype));
		}
	};
}

TEST_CASE("unknown prototype fails", "[drl][JobCreationService]")
{
	Fixture f;
	f.terrain.digShaft(0);
	f.terrain.initialiseTile(0, 1);
	REQUIRE_FALSE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(1, 0)));
	REQUIRE(f.jobData.jobs.empty());
}

TEST_CASE("missing tile fails", "[drl][JobCreationService]")
{
	Fixture f;
	f.registerDigJob();
	REQUIRE_FALSE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(1, 0)));
	REQUIRE(f.jobData.jobs.empty());
}

TEST_CASE("creates job on a real tile left and right", "[drl][JobCreationService]")
{
	Fixture f;
	f.registerDigJob(2.5f);
	REQUIRE(f.terrain.digShaft(0));
	f.terrain.initialiseTile(0, 1);
	f.terrain.initialiseTile(0, -1);

	REQUIRE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(1, 0)));
	REQUIRE(f.terrain.getTile(0, 1).jobReserved);
	REQUIRE(f.jobData.jobs.size() == 1);
	REQUIRE(f.jobData.jobs[0].tile == glm::ivec2(1, 0));
	REQUIRE(f.jobData.jobs[0].work == 2.5f);
	REQUIRE(f.jobData.jobs[0].prototypeId == jobPrototypeIdFromName("Job_Dig"));

	REQUIRE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(-1, 0)));
	REQUIRE(f.terrain.getTile(0, -1).jobReserved);
	REQUIRE(f.jobData.jobs.size() == 2);
	REQUIRE(f.jobData.jobs[1].tile == glm::ivec2(-1, 0));
}

TEST_CASE("second create on same tile fails", "[drl][JobCreationService]")
{
	Fixture f;
	f.registerDigJob();
	REQUIRE(f.terrain.digShaft(0));
	f.terrain.initialiseTile(0, 1);
	REQUIRE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(1, 0)));
	REQUIRE_FALSE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(1, 0)));
	REQUIRE(f.jobData.jobs.size() == 1);
}

TEST_CASE("calculateOffset is applied", "[drl][JobCreationService]")
{
	Fixture f;
	JobPrototype prototype{};
	prototype.name = "Job_Dig";
	prototype.work = 1.0f;
	prototype.calculateOffset = [](const drl::JobInstance&, const JobPrototype&)
	{
		return glm::vec2(0.25f, 0.5f);
	};
	f.prototypes.registerPrototype(std::move(prototype));
	REQUIRE(f.terrain.digShaft(0));
	f.terrain.initialiseTile(0, 2);

	REQUIRE(f.jobs.createJob(jobPrototypeIdFromName("Job_Dig"), 0, glm::ivec2(2, 0)));
	REQUIRE_THAT(f.jobData.jobs[0].offset.x, Catch::Matchers::WithinAbs(0.25f, 0.0001f));
	REQUIRE_THAT(f.jobData.jobs[0].offset.y, Catch::Matchers::WithinAbs(0.5f, 0.0001f));
}
