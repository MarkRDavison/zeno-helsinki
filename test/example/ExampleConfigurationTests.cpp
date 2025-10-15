#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <helsinki/Example/ExampleConfiguration.hpp>

namespace ex
{
	namespace test
	{

		TEST_CASE("an example test works", "[ExampleConfiguration]")
		{
			REQUIRE(1 == 1);
		}

	}
}