#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Worker.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{
namespace WorkerRecruitmentServiceTests
{

	struct Fixture
	{
		WorkerData workerData;
		WorkerPrototypeService workerPrototypes;
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };

		Fixture()
		{
			WorkerPrototype builder{};
			builder.name = "Worker_Builder";
			workerPrototypes.registerPrototype(std::move(builder));
			WorkerPrototype miner{};
			miner.name = "Worker_Miner";
			workerPrototypes.registerPrototype(std::move(miner));
		}
	};

	TEST_CASE("unregistered type has required count 0", "[drl][WorkerRecruitmentService]")
	{
		Fixture f;
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 0);
		REQUIRE(f.recruitment.getRequiredWorkerCount(prototypeIdFromName("Worker_Builder")) == 0);
		REQUIRE(f.recruitment.getRequiredWorkerTypes().empty());
	}

	TEST_CASE("register creates then adds", "[drl][WorkerRecruitmentService]")
	{
		Fixture f;
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Builder", 2);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 2);
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Builder", 3);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 5);
		f.recruitment.registerWorkerPrototypeRequirement(prototypeIdFromName("Worker_Miner"), 1);
		REQUIRE(f.recruitment.getRequiredWorkerCount(prototypeIdFromName("Worker_Miner")) == 1);
	}

	TEST_CASE("reduce insufficient is a no-op", "[drl][WorkerRecruitmentService]")
	{
		Fixture f;
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Builder", 2);
		f.recruitment.reduceWorkerPrototypeRequirement("Worker_Builder", 3);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 2);
		f.recruitment.reduceWorkerPrototypeRequirement(prototypeIdFromName("Worker_Builder"), 5);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 2);
	}

	TEST_CASE("reduce subtracts when enough demand exists", "[drl][WorkerRecruitmentService]")
	{
		Fixture f;
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Builder", 4);
		f.recruitment.reduceWorkerPrototypeRequirement("Worker_Builder", 1);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 3);
		f.recruitment.reduceWorkerPrototypeRequirement(prototypeIdFromName("Worker_Builder"), 3);
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 0);
	}

	TEST_CASE("getRequiredWorkerTypes includes only positive counts", "[drl][WorkerRecruitmentService]")
	{
		Fixture f;
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Builder", 2);
		f.recruitment.registerWorkerPrototypeRequirement("Worker_Miner", 1);
		const auto both = f.recruitment.getRequiredWorkerTypes();
		REQUIRE(both.contains(prototypeIdFromName("Worker_Builder")));
		REQUIRE(both.contains(prototypeIdFromName("Worker_Miner")));

		f.recruitment.reduceWorkerPrototypeRequirement("Worker_Miner", 1);
		const auto buildersOnly = f.recruitment.getRequiredWorkerTypes();
		REQUIRE(buildersOnly.contains(prototypeIdFromName("Worker_Builder")));
		REQUIRE_FALSE(buildersOnly.contains(prototypeIdFromName("Worker_Miner")));
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Miner") == 0);
	}

}
}
