#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <helsinki/Physics/Physics.hpp>
#include <glm/gtc/quaternion.hpp>

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

	BodyId addStaticBox(World& world, glm::vec3 halfExtents, glm::vec3 position, glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f))
	{
		BodyDesc box;
		box.shape.kind = Shape::Kind::Box;
		box.shape.halfExtents = halfExtents;
		box.motion = MotionType::Static;
		box.layer = Layer::NonMoving;
		box.pose.position = position;
		box.pose.rotation = rotation;
		const BodyId id = world.createBody(box);
		REQUIRE(id.valid());
		return id;
	}

	void stepUntil(World& world, int frames)
	{
		const float dt = 1.f / 60.f;
		for (int i = 0; i < frames; ++i)
		{
			world.step(dt);
		}
	}

	TEST_CASE_METHOD(WorldFixture, "character land jump slope climb and 2D lock", "[Physics]")
	{
		SECTION("land jump climb and slopes")
		{
			World world(context, WorldSettings{});
			addStaticBox(world, {20.f, 0.5f, 20.f}, {0.f, -0.5f, 0.f});

			CharacterDesc desc;
			desc.pose.position = {0.f, 3.f, 0.f};
			desc.maxSlopeAngleDeg = 45.f;
			desc.jumpSpeed = 6.f;
			const CharacterId id = world.createCharacter(desc);
			REQUIRE(id.valid());
			REQUIRE_FALSE(world.isGrounded(id));
			REQUIRE_FALSE(world.isClimbing(id));

			bool landed = false;
			const float dt = 1.f / 60.f;
			for (int i = 0; i < 180; ++i)
			{
				world.step(dt);
				if (world.isGrounded(id))
				{
					landed = true;
					break;
				}
			}
			REQUIRE(landed);
			REQUIRE(world.getPose(id).position.y == Catch::Approx(0.f).margin(0.15f));

			world.jump(id);
			world.step(dt);
			world.step(dt);
			REQUIRE_FALSE(world.isGrounded(id));

			landed = false;
			for (int i = 0; i < 180; ++i)
			{
				world.step(dt);
				if (world.isGrounded(id))
				{
					landed = true;
					break;
				}
			}
			REQUIRE(landed);

			world.setClimbing(id, true);
			REQUIRE(world.isClimbing(id));
			world.jump(id);
			world.step(dt);
			REQUIRE(world.isGrounded(id));
			REQUIRE(world.isClimbing(id));
			world.setClimbing(id, false);

			const glm::quat gentle = glm::angleAxis(glm::radians(25.f), glm::vec3(0.f, 0.f, 1.f));
			addStaticBox(world, {4.f, 0.2f, 2.f}, {5.f, 1.9f, 0.f}, gentle);
			const float startX = world.getPose(id).position.x;
			const float startY = world.getPose(id).position.y;
			world.setMove(id, {4.f, 0.f, 0.f});
			bool climbed = false;
			for (int i = 0; i < 120; ++i)
			{
				world.step(dt);
				const Pose pose = world.getPose(id);
				if (pose.position.x > startX + 1.f && pose.position.y > startY + 0.3f && world.isGrounded(id))
				{
					climbed = true;
					break;
				}
			}
			REQUIRE(climbed);
			world.setMove(id, {0.f, 0.f, 0.f});
		}

		SECTION("steep slope blocks walk")
		{
			World world(context, WorldSettings{});
			addStaticBox(world, {20.f, 0.5f, 20.f}, {0.f, -0.5f, 0.f});
			const glm::quat steep = glm::angleAxis(glm::radians(70.f), glm::vec3(0.f, 0.f, 1.f));
			addStaticBox(world, {2.f, 0.2f, 2.f}, {2.5f, 2.f, 0.f}, steep);

			CharacterDesc desc;
			desc.pose.position = {0.f, 3.f, 0.f};
			desc.maxSlopeAngleDeg = 45.f;
			const CharacterId id = world.createCharacter(desc);
			REQUIRE(id.valid());

			bool landed = false;
			const float dt = 1.f / 60.f;
			for (int i = 0; i < 180; ++i)
			{
				world.step(dt);
				if (world.isGrounded(id))
				{
					landed = true;
					break;
				}
			}
			REQUIRE(landed);

			world.setMove(id, {4.f, 0.f, 0.f});
			stepUntil(world, 180);
			REQUIRE(world.getPose(id).position.y < 0.8f);
			REQUIRE(world.getPose(id).position.x < 3.5f);
		}

		SECTION("2D character stays at z 0")
		{
			World world(context, WorldSettings{ .dim = Dim::D2 });
			addStaticBox(world, {20.f, 0.5f, 5.f}, {0.f, -0.5f, 0.f});

			CharacterDesc desc;
			desc.dim = Dim::D2;
			desc.pose.position = {0.f, 3.f, 2.f};
			const CharacterId id = world.createCharacter(desc);
			REQUIRE(id.valid());
			REQUIRE(world.getPose(id).position.z == Catch::Approx(0.f).margin(1.e-3f));

			stepUntil(world, 120);
			REQUIRE(world.getPose(id).position.z == Catch::Approx(0.f).margin(1.e-3f));
		}
	}
}
