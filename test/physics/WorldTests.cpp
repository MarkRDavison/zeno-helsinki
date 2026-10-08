#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <helsinki/Physics/Physics.hpp>
#include <cmath>

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

	TEST_CASE_METHOD(WorldFixture, "3D dynamic sphere falls onto static box", "[Physics]")
	{
		World world(context, WorldSettings{});

		BodyDesc floor;
		floor.shape.kind = Shape::Kind::Box;
		floor.shape.halfExtents = {10.f, 0.5f, 10.f};
		floor.motion = MotionType::Static;
		floor.layer = Layer::NonMoving;
		floor.pose.position = {0.f, -0.5f, 0.f};
		const BodyId floorId = world.createBody(floor);
		REQUIRE(floorId.valid());

		BodyDesc sphere;
		sphere.shape.kind = Shape::Kind::Sphere;
		sphere.shape.radius = 0.5f;
		sphere.motion = MotionType::Dynamic;
		sphere.layer = Layer::Moving;
		sphere.pose.position = {0.f, 2.f, 0.f};
		const BodyId sphereId = world.createBody(sphere);
		REQUIRE(sphereId.valid());

		const float startY = world.getPose(sphereId).position.y;
		const float dt = 1.f / 60.f;
		for (int i = 0; i < 10; ++i)
		{
			world.step(dt);
		}
		const float midY = world.getPose(sphereId).position.y;
		REQUIRE(midY < startY);

		for (int i = 0; i < 180; ++i)
		{
			world.step(dt);
		}
		const float settledY = world.getPose(sphereId).position.y;
		REQUIRE(settledY > 0.4f);
		REQUIRE(settledY < 1.0f);
	}

	TEST_CASE_METHOD(WorldFixture, "2D dynamic box stays in XY plane", "[Physics]")
	{
		World world(context, WorldSettings{ .dim = Dim::D2 });

		BodyDesc floor;
		floor.shape.kind = Shape::Kind::Box;
		floor.shape.halfExtents = {10.f, 0.5f, 5.f};
		floor.motion = MotionType::Static;
		floor.layer = Layer::NonMoving;
		floor.pose.position = {0.f, -0.5f, 0.f};
		REQUIRE(world.createBody(floor).valid());

		BodyDesc box;
		box.shape.kind = Shape::Kind::Box;
		box.shape.halfExtents = {0.5f, 0.5f, 4.f};
		box.motion = MotionType::Dynamic;
		box.layer = Layer::Moving;
		box.pose.position = {0.5f, 2.f, 3.f};
		const BodyId boxId = world.createBody(box);
		REQUIRE(boxId.valid());
		REQUIRE(world.getPose(boxId).position.z == Catch::Approx(0.f).margin(1.e-4f));

		world.setLinearVelocity(boxId, {1.f, 0.f, 8.f});
		const float dt = 1.f / 60.f;
		for (int i = 0; i < 180; ++i)
		{
			world.step(dt);
		}

		const Pose pose = world.getPose(boxId);
		REQUIRE(pose.position.z == Catch::Approx(0.f).margin(1.e-3f));
		REQUIRE(std::abs(pose.rotation.x) == Catch::Approx(0.f).margin(1.e-3f));
		REQUIRE(std::abs(pose.rotation.y) == Catch::Approx(0.f).margin(1.e-3f));
	}

	TEST_CASE_METHOD(WorldFixture, "create and destroy hundreds of bodies", "[Physics]")
	{
		World world(context, WorldSettings{});
		BodyId ids[256];
		for (int i = 0; i < 256; ++i)
		{
			BodyDesc desc;
			desc.shape.kind = Shape::Kind::Box;
			desc.shape.halfExtents = {0.25f, 0.25f, 0.25f};
			desc.motion = MotionType::Dynamic;
			desc.layer = Layer::Moving;
			desc.pose.position = {static_cast<float>(i) * 0.6f, 4.f, 0.f};
			ids[i] = world.createBody(desc);
			REQUIRE(ids[i].valid());
		}

		for (int i = 0; i < 256; ++i)
		{
			world.destroyBody(ids[i]);
			REQUIRE(world.getPose(ids[i]).position.y == Catch::Approx(0.f));
		}
	}
}
