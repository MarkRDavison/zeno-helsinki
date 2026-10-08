#include <catch2/catch_test_macros.hpp>
#include <helsinki/Physics/Physics.hpp>

#include <thread>

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

	TEST_CASE_METHOD(WorldFixture, "Moving vs NonMoving fires contact callback", "[Physics]")
	{
		World world(context, WorldSettings{});
		const std::thread::id caller = std::this_thread::get_id();
		int contacts = 0;
		bool insideStep = true;
		world.setContactCallback([&](const Contact&)
		{
			REQUIRE(std::this_thread::get_id() == caller);
			REQUIRE(insideStep);
			++contacts;
		});

		BodyDesc floor;
		floor.shape.kind = Shape::Kind::Box;
		floor.shape.halfExtents = {10.f, 0.5f, 10.f};
		floor.motion = MotionType::Static;
		floor.layer = Layer::NonMoving;
		floor.pose.position = {0.f, -0.5f, 0.f};
		REQUIRE(world.createBody(floor).valid());

		BodyDesc sphere;
		sphere.shape.kind = Shape::Kind::Sphere;
		sphere.shape.radius = 0.5f;
		sphere.motion = MotionType::Dynamic;
		sphere.layer = Layer::Moving;
		sphere.pose.position = {0.f, 2.f, 0.f};
		REQUIRE(world.createBody(sphere).valid());

		const float dt = 1.f / 60.f;
		for (int i = 0; i < 180; ++i)
		{
			world.step(dt);
		}
		insideStep = false;
		REQUIRE(contacts > 0);
	}

	TEST_CASE_METHOD(WorldFixture, "NonMoving vs NonMoving does not fire contact callback", "[Physics]")
	{
		World world(context, WorldSettings{});
		int contacts = 0;
		world.setContactCallback([&](const Contact&)
		{
			++contacts;
		});

		BodyDesc a;
		a.shape.kind = Shape::Kind::Box;
		a.shape.halfExtents = {1.f, 1.f, 1.f};
		a.motion = MotionType::Static;
		a.layer = Layer::NonMoving;
		a.pose.position = {0.f, 0.f, 0.f};
		REQUIRE(world.createBody(a).valid());

		BodyDesc b;
		b.shape.kind = Shape::Kind::Box;
		b.shape.halfExtents = {1.f, 1.f, 1.f};
		b.motion = MotionType::Static;
		b.layer = Layer::NonMoving;
		b.pose.position = {0.5f, 0.f, 0.f};
		REQUIRE(world.createBody(b).valid());

		world.step(1.f / 60.f);
		REQUIRE(contacts == 0);
	}

	TEST_CASE_METHOD(WorldFixture, "Sensor overlap does not shove dynamic and callbacks flush in step", "[Physics]")
	{
		World world(context, WorldSettings{ .gravity = {0.f, 0.f, 0.f} });
		const std::thread::id caller = std::this_thread::get_id();
		int contacts = 0;
		bool insideStep = true;
		world.setContactCallback([&](const Contact&)
		{
			REQUIRE(std::this_thread::get_id() == caller);
			REQUIRE(insideStep);
			++contacts;
		});

		BodyDesc sensor;
		sensor.shape.kind = Shape::Kind::Box;
		sensor.shape.halfExtents = {0.5f, 0.5f, 0.5f};
		sensor.motion = MotionType::Static;
		sensor.layer = Layer::Sensor;
		sensor.sensor = true;
		sensor.pose.position = {2.f, 0.5f, 0.f};
		REQUIRE(world.createBody(sensor).valid());

		BodyDesc mover;
		mover.shape.kind = Shape::Kind::Box;
		mover.shape.halfExtents = {0.25f, 0.25f, 0.25f};
		mover.motion = MotionType::Dynamic;
		mover.layer = Layer::Moving;
		mover.pose.position = {0.f, 0.5f, 0.f};
		const BodyId moverId = world.createBody(mover);
		REQUIRE(moverId.valid());
		world.setLinearVelocity(moverId, {4.f, 0.f, 0.f});

		const float dt = 1.f / 60.f;
		for (int i = 0; i < 60; ++i)
		{
			world.step(dt);
		}
		insideStep = false;

		REQUIRE(contacts > 0);
		const float x = world.getPose(moverId).position.x;
		REQUIRE(x > 2.f);
	}
}
