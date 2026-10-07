#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Combat.hpp>
#include <unordered_map>

TEST_CASE("neutral resist takes full damage", "[tower][combat]")
{
	CHECK(tower::takenDamage(10.0f, 0.0f) == Catch::Approx(10.0f));
}

TEST_CASE("resist 0.2 removes 20 percent", "[tower][combat]")
{
	CHECK(tower::takenDamage(10.0f, 0.2f) == Catch::Approx(8.0f));
}

TEST_CASE("negative resist is innate weakness", "[tower][combat]")
{
	CHECK(tower::takenDamage(10.0f, -0.2f) == Catch::Approx(12.0f));
}

TEST_CASE("resist 1.0 is immune without warn", "[tower][combat]")
{
	const auto clamped = tower::clampResist(1.0f);
	CHECK(clamped.value == Catch::Approx(1.0f));
	CHECK_FALSE(clamped.warned);
	CHECK(tower::takenDamage(10.0f, 1.0f) == Catch::Approx(0.0f));
}

TEST_CASE("resist above 1 clamps and warns", "[tower][combat]")
{
	const auto clamped = tower::clampResist(1.5f);
	CHECK(clamped.value == Catch::Approx(1.0f));
	CHECK(clamped.warned);
	CHECK(tower::takenDamage(10.0f, 1.5f) == Catch::Approx(0.0f));
}

TEST_CASE("missing resist key is 0", "[tower][combat]")
{
	const std::unordered_map<std::string, float> resist{ { "physical", 0.25f } };
	CHECK(tower::resistOf(resist, "fire") == Catch::Approx(0.0f));
	CHECK(tower::resistOf({}, "physical") == Catch::Approx(0.0f));
	CHECK(tower::takenDamage(10.0f, tower::resistOf(resist, "fire")) == Catch::Approx(10.0f));
}

TEST_CASE("tank physical 0.25 does not floor", "[tower][combat]")
{
	CHECK(tower::takenDamage(1.0f, 0.25f) == Catch::Approx(0.75f));
}

TEST_CASE("kill when current is at or below 0", "[tower][combat]")
{
	float current = 0.75f;
	tower::applyHit(current, 1.0f, 0.25f);
	CHECK(current == Catch::Approx(0.0f));
	CHECK(tower::isDead(current));

	current = 0.76f;
	tower::applyHit(current, 1.0f, 0.25f);
	CHECK(current == Catch::Approx(0.01f));
	CHECK_FALSE(tower::isDead(current));
}

TEST_CASE("overkill does not heal", "[tower][combat]")
{
	float current = 1.0f;
	tower::applyHit(current, 10.0f, 0.0f);
	CHECK(current <= 0.0f);
	CHECK(tower::isDead(current));
	CHECK(tower::takenDamage(10.0f, 2.0f) == Catch::Approx(0.0f));
}
