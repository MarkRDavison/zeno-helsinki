#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace SystemGlanceTests
{

TEST_CASE("starter map still has Helion yard and Kestrel factory in data", "[slr][SystemGlance]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(yardAtSystem(world, HelionSystemId) != nullptr);
	bool kestrelHasFactory = false;
	for (const auto& factory : world.factories)
	{
		if (factory.systemId == KestrelSystemId)
		{
			kestrelHasFactory = true;
			break;
		}
	}

	REQUIRE(kestrelHasFactory);
	bool rimwardHasMine = false;
	for (const auto& mine : world.mines)
	{
		if (mine.systemId == 5)
		{
			rimwardHasMine = true;
			break;
		}
	}

	REQUIRE(rimwardHasMine);
}

}
}
