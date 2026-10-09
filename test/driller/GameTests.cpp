#include <catch2/catch_test_macros.hpp>
#include <Core/Game.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/TerrainAlterationService.hpp>

using drl::EconomyResourceService;
using drl::Game;
using drl::GameCommandService;
using drl::IGameTickService;
using drl::TerrainAlterationService;
using drl::TerrainData;

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
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
		GameCommandService commands{ terrain, economy };
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
