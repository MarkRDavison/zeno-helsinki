#include <catch2/catch_test_macros.hpp>
#include <helsinki/System/Utils/SplitMix64.hpp>

namespace hl
{
namespace Test
{

TEST_CASE("same seed produces the same sequence", "[Utility][SplitMix64]")
{
	std::uint64_t a = 1;
	std::uint64_t b = 1;
	REQUIRE(splitMix64(a) == splitMix64(b));
	REQUIRE(splitMix64(a) == splitMix64(b));
	REQUIRE(a == b);
}

TEST_CASE("splitMix64Unit stays in unit interval", "[Utility][SplitMix64]")
{
	std::uint64_t state = 42;
	for (int i = 0; i < 64; ++i)
	{
		const float unit = splitMix64Unit(state);
		REQUIRE(unit >= 0.0f);
		REQUIRE(unit < 1.0f);
	}
}

TEST_CASE("splitMix64Seed folds any number of parts", "[Utility][SplitMix64]")
{
	REQUIRE(splitMix64Seed({}) == 0);
	REQUIRE(splitMix64Seed({ 1, 2, 3 }) == splitMix64Seed({ 1, 2, 3 }));
	REQUIRE(splitMix64Seed({ 1, 2, 3 }) != splitMix64Seed({ 1, 2 }));
	REQUIRE(splitMix64Seed({ 1, 2, 3 }) != splitMix64Seed({ 3, 2, 1 }));
}

}
}
