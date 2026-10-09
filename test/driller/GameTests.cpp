#include <catch2/catch_test_macros.hpp>
#include <Core/Game.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{
namespace GameTests
{

class RecordingTickService : public IGameTickService
	{
	public:
		void update(float delta) override
		{
			lastDelta = delta;
			++updateCount;
		}

		float lastDelta{ 0.0f };
		int updateCount{ 0 };
	};

	struct Fixture
	{
		TerrainData data;
		JobData jobData;
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
		JobPrototypeService prototypes;
		JobCreationService jobCreation{ jobData, prototypes, terrain };
		WorkerData workerData;
		WorkerPrototypeService workerPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes };
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		BuildingPlacementService buildings{ buildingData, terrain, recruitment, jobCreation, buildingPrototypes };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings };
};

TEST_CASE("Game::update scales tick delta by SimSpeed", "[drl][Game]")
{
	Fixture f;
	float simSpeed = 2.0f;
	Game game(f.commands, simSpeed);
	RecordingTickService tick;
	game.addTickService(tick);

	game.update(0.25f);

	REQUIRE(tick.updateCount == 1);
	REQUIRE(tick.lastDelta == 0.5f);
	REQUIRE(f.commands.currentTick() == 1);
}

TEST_CASE("Game::update default SimSpeed is 1", "[drl][Game]")
{
	Fixture f;
	float simSpeed = 1.0f;
	Game game(f.commands, simSpeed);
	RecordingTickService tick;
	game.addTickService(tick);

	game.update(0.1f);

	REQUIRE(tick.lastDelta == 0.1f);
	REQUIRE(f.commands.currentTick() == 1);
}

TEST_CASE("Game::update still ticks commands with no tick services", "[drl][Game]")
{
	Fixture f;
	float simSpeed = 4.0f;
	Game game(f.commands, simSpeed);
	game.update(1.0f);
	game.update(1.0f);
	REQUIRE(f.commands.currentTick() == 2);
}

}
}
