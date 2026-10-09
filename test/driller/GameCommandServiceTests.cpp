#include <catch2/catch_test_macros.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/TerrainAlterationService.hpp>

using drl::CommandContext;
using drl::CommandSource;
using drl::EconomyResourceService;
using drl::GameCommand;
using drl::GameCommandService;
using drl::ResourceMoney;
using drl::ResourceOre;
using drl::TerrainAlterationService;
using drl::TerrainData;

namespace
{
	struct Fixture
	{
		TerrainData data;
		TerrainAlterationService terrain{ data };
		EconomyResourceService economy;
		GameCommandService commands{ terrain, economy };

		Fixture()
		{
			economy.setMax(ResourceOre, -1);
			economy.set(ResourceOre, 0);
			economy.setMax(ResourceMoney, -1);
			economy.set(ResourceMoney, 500);
		}
	};
}

TEST_CASE("tick is monotonic from 0", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.currentTick() == 0);
	f.commands.tick();
	REQUIRE(f.commands.currentTick() == 1);
	f.commands.tick();
	REQUIRE(f.commands.currentTick() == 2);
}

TEST_CASE("player DigShaft skip-level does not pay", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE_FALSE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Player, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == -1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("player DigShaft cannot afford does not dig", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	f.economy.set(ResourceMoney, 50);
	REQUIRE_FALSE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Player, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 0);
	REQUIRE(f.economy.get(ResourceMoney) == 50);
}

TEST_CASE("player DigShaft pays 100 times level then digs", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Player, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 400);
}

TEST_CASE("setup DigShaft next level does not charge", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.execute(GameCommand::digShaft(0, CommandSource::Setup, CommandContext::DiggingShaft)));
	REQUIRE(f.commands.execute(GameCommand::digShaft(1, CommandSource::Setup, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("system DigShaft next level does not charge", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.execute(GameCommand::digShaft(0, CommandSource::System, CommandContext::DiggingShaft)));
	REQUIRE(f.commands.execute(GameCommand::digShaft(1, CommandSource::System, CommandContext::DiggingShaft)));
	REQUIRE(f.data.shaftLevel == 1);
	REQUIRE(f.economy.get(ResourceMoney) == 500);
}

TEST_CASE("DigTile succeeds and refuses non-contiguous on the right", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.commands.execute(GameCommand::digTile(0, 1, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE(f.terrain.isTileDugOut(0, 1));
	REQUIRE_FALSE(f.commands.execute(GameCommand::digTile(0, 3, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE_FALSE(f.terrain.doesTileExist(0, 3));
}

TEST_CASE("DigTile succeeds and refuses non-contiguous on the left", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.terrain.digShaft(0));
	REQUIRE(f.commands.execute(GameCommand::digTile(0, -1, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE(f.terrain.isTileDugOut(0, -1));
	REQUIRE_FALSE(f.commands.execute(GameCommand::digTile(0, -3, CommandSource::Player, CommandContext::DiggingTile)));
	REQUIRE_FALSE(f.terrain.doesTileExist(0, -3));
}

TEST_CASE("AddResource increases ore and money", "[drl][GameCommandService]")
{
	Fixture f;
	REQUIRE(f.commands.execute(GameCommand::addResource(ResourceOre, 12, CommandSource::System, CommandContext::AddResource)));
	REQUIRE(f.commands.execute(GameCommand::addResource(ResourceMoney, 25, CommandSource::System, CommandContext::AddResource)));
	REQUIRE(f.economy.get(ResourceOre) == 12);
	REQUIRE(f.economy.get(ResourceMoney) == 525);
}
