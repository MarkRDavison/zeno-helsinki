#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Shuttle.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>

namespace drl
{
namespace ShuttleCreationServiceTests
{

	TEST_CASE("unknown prototype fails", "[drl][ShuttleCreationService]")
	{
		ShuttleData data;
		ShuttlePrototypeService prototypes;
		ShuttleCreationService service{ data, prototypes };
		REQUIRE_FALSE(service.createShuttle(prototypeIdFromName("Shuttle_Basic")));
		REQUIRE(data.shuttles.empty());
	}

	TEST_CASE("creates shuttle idle at start pose", "[drl][ShuttleCreationService]")
	{
		ShuttleData data;
		ShuttlePrototypeService prototypes;
		ShuttleCreationService service{ data, prototypes };
		ShuttlePrototype prototype{};
		prototype.name = "Shuttle_Basic";
		prototypes.registerPrototype(std::move(prototype));

		REQUIRE(service.createShuttle(prototypeIdFromName("Shuttle_Basic")));
		REQUIRE(data.shuttles.size() == 1);
		REQUIRE(data.shuttles[0].id != 0);
		REQUIRE(data.shuttles[0].prototypeId == prototypeIdFromName("Shuttle_Basic"));
		REQUIRE(data.shuttles[0].state == ShuttleState::Idle);
		REQUIRE(data.shuttles[0].elapsed == 0.0f);
		REQUIRE(data.shuttles[0].position == kShuttleStartingPosition);
		REQUIRE(data.shuttles[0].startingPosition == kShuttleStartingPosition);
		REQUIRE(data.shuttles[0].surfacePosition == kShuttleSurfacePosition);
		REQUIRE(data.shuttles[0].leavingPosition == kShuttleLeavingPosition);
	}

}
}
