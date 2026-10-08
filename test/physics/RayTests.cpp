#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <helsinki/Physics/Physics.hpp>

namespace hl::physics::test
{
	struct WorldFixture
	{
		Context context;

		WorldFixture()
		{
			REQUIRE(context.init());
		}

		~WorldFixture()
		{
			context.shutdown();
		}
	};

	TEST_CASE_METHOD(WorldFixture, "castRay hits known geometry", "[Physics]")
	{
		World world(context, WorldSettings{});

		BodyDesc box;
		box.shape.kind = Shape::Kind::Box;
		box.shape.halfExtents = {0.5f, 0.5f, 0.5f};
		box.motion = MotionType::Static;
		box.layer = Layer::NonMoving;
		box.pose.position = {0.f, 0.f, 0.f};
		const BodyId id = world.createBody(box);
		REQUIRE(id.valid());

		const auto hit = world.castRay({0.f, 2.f, 0.f}, {0.f, -4.f, 0.f});
		REQUIRE(hit.has_value());
		REQUIRE(hit->body == id);
		REQUIRE(hit->fraction > 0.f);
		REQUIRE(hit->fraction <= 1.f);
		REQUIRE(hit->point.y == Catch::Approx(0.5f).margin(0.05f));
	}

	TEST_CASE_METHOD(WorldFixture, "castRay misses after destroyBody", "[Physics]")
	{
		World world(context, WorldSettings{});

		BodyDesc box;
		box.shape.kind = Shape::Kind::Box;
		box.shape.halfExtents = {0.5f, 0.5f, 0.5f};
		box.motion = MotionType::Static;
		box.layer = Layer::NonMoving;
		const BodyId id = world.createBody(box);
		REQUIRE(id.valid());
		REQUIRE(world.castRay({0.f, 2.f, 0.f}, {0.f, -4.f, 0.f}).has_value());

		world.destroyBody(id);
		REQUIRE_FALSE(world.castRay({0.f, 2.f, 0.f}, {0.f, -4.f, 0.f}).has_value());
	}
}
