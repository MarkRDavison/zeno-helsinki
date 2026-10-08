#include <catch2/catch_test_macros.hpp>
#include <helsinki/Physics/Physics.hpp>

#include <stdexcept>

namespace hl::physics::test
{
	struct ContextFixture
	{
		Context context;
	};

	TEST_CASE_METHOD(ContextFixture, "Context init shutdown and once-only", "[Physics]")
	{
		REQUIRE(context.init());
		REQUIRE(context.ok());
		REQUIRE_THROWS_AS(context.init(), std::runtime_error);
		context.shutdown();
		REQUIRE_FALSE(context.ok());
	}
}
