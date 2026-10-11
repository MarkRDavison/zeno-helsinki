#include <Views/TimeChrome.hpp>
#include <World/GalaxyDate.hpp>
#include <World/SimClock.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace SimClockTests
{

TEST_CASE("paused clock emits no ticks", "[slr][SimClock]")
{
	SimClock clock;
	REQUIRE(clock.paused());
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds) == 0);
	REQUIRE(clock.takeTicks(10.0f) == 0);
}

TEST_CASE("1x emits one tick per period and keeps leftover", "[slr][SimClock]")
{
	SimClock clock;
	clock.setPaused(false);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds) == 1);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds * 0.5f) == 0);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds * 0.5f) == 1);
}

TEST_CASE("2x waits half as long as 1x", "[slr][SimClock]")
{
	SimClock clock;
	clock.setPaused(false);
	clock.setSpeed(2);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds * 0.5f) == 1);
}

TEST_CASE("pause keeps leftover then unpause continues", "[slr][SimClock]")
{
	SimClock clock;
	clock.setPaused(false);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds * 0.5f) == 0);
	clock.setPaused(true);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds) == 0);
	clock.setPaused(false);
	REQUIRE(clock.takeTicks(SimClock::TickPeriodSeconds * 0.5f) == 1);
}

TEST_CASE("huge dt still one tick this call", "[slr][SimClock]")
{
	SimClock clock;
	clock.setPaused(false);
	REQUIRE(clock.takeTicks(100.0f) == 1);
}

TEST_CASE("formatGalaxyDate epoch and month roll", "[slr][GalaxyDate]")
{
	REQUIRE(formatGalaxyDate(0) == "2200-01-01");
	REQUIRE(formatGalaxyDate(1) == "2200-01-02");
	REQUIRE(formatGalaxyDate(31) == "2200-02-01");
}

TEST_CASE("formatGalaxyDate year roll on non-leap 2200", "[slr][GalaxyDate]")
{
	REQUIRE(formatGalaxyDate(365) == "2201-01-01");
}

TEST_CASE("active time control is pause when paused", "[slr][TimeChrome]")
{
	REQUIRE(activeTimeControl(true, 4) == TimeSpeedControl::Pause);
	REQUIRE(activeTimeControl(false, 1) == TimeSpeedControl::Play);
	REQUIRE(activeTimeControl(false, 2) == TimeSpeedControl::Speed2);
	REQUIRE(activeTimeControl(false, 4) == TimeSpeedControl::Speed4);
}

}
}
