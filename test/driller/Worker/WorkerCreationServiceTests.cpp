#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Worker.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/System/glm.hpp>

using drl::WorkerCreationService;
using drl::WorkerData;
using drl::WorkerPrototype;
using drl::WorkerPrototypeService;
using drl::WorkerState;
using drl::prototypeIdFromName;

TEST_CASE("unknown prototype fails", "[drl][WorkerCreationService]")
{
	WorkerData data;
	WorkerPrototypeService prototypes;
	WorkerCreationService service{ data, prototypes };
	REQUIRE_FALSE(service.createWorker(prototypeIdFromName("Worker_Builder"), glm::vec2(1.0f, 0.0f)));
	REQUIRE(data.workers.empty());
}

TEST_CASE("creates worker at coordinates", "[drl][WorkerCreationService]")
{
	WorkerData data;
	WorkerPrototypeService prototypes;
	WorkerCreationService service{ data, prototypes };
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
