#include <World/StarterMap.hpp>
#include <World/Goods.hpp>
#include <utility>

namespace slr
{
namespace
{

	void addLane(WorldData& world, long long a, long long b)
	{
		if (a > b)
		{
			std::swap(a, b);
		}

		world.hyperlanes.push_back(Hyperlane{ .a = a, .b = b });
	}

}

	WorldData makeStarterMap()
	{
		WorldData world;

		world.systems = {
			StarSystem{ .id = 1, .name = "Helion", .position = { 120.0f, 280.0f } },
			StarSystem{ .id = 2, .name = "Kestrel", .position = { 340.0f, 180.0f } },
			StarSystem{ .id = 3, .name = "Junction", .position = { 520.0f, 320.0f } },
			StarSystem{ .id = 4, .name = "Ashreach", .position = { 480.0f, 520.0f } },
			StarSystem{ .id = 5, .name = "Rimward", .position = { 760.0f, 140.0f } },
			StarSystem{ .id = 6, .name = "Caldera", .position = { 820.0f, 380.0f } },
			StarSystem{ .id = 7, .name = "Nadir", .position = { 980.0f, 260.0f } },
			StarSystem{ .id = 8, .name = "Vesper", .position = { 700.0f, 560.0f } },
		};

		addLane(world, 1, 2);
		addLane(world, 2, 3);
		addLane(world, 3, 4);
		addLane(world, 3, 5);
		addLane(world, 3, 6);
		addLane(world, 5, 7);
		addLane(world, 6, 7);
		addLane(world, 4, 8);
		addLane(world, 6, 8);

		StarSystem* rimward = world.systemById(5);
		rimward->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0 });
		world.mines.push_back(Mine{ .systemId = 5, .goodId = SteelGoodId, .outputPerTick = 100 });

		return world;
	}

}
