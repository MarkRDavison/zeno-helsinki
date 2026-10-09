#include <catch2/catch_test_macros.hpp>
#include <Core/Game.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>

using drl::EconomyResourceService;
using drl::Game;
using drl::GameCommandService;
using drl::IGameTickService;
using drl::JobCreationService;
using drl::JobData;
using drl::JobPrototypeService;
using drl::TerrainAlterationService;
using drl::TerrainData;
using drl::WorkerCreationService;
using drl::WorkerData;
using drl::WorkerPrototypeService;

namespace
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
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation };
	};
}

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
