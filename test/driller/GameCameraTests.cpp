#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Views/GameCamera.hpp>

namespace drl
{
namespace GameCameraTests
{

TEST_CASE("screenToWorld is identity at default zoom and pan", "[drl][GameCamera]")
{
	GameCamera camera;
	const glm::vec2 screen{ 640.0f, 64.0f };
	const glm::vec2 world = camera.screenToWorld(screen);
	REQUIRE(world.x == Catch::Approx(screen.x));
	REQUIRE(world.y == Catch::Approx(screen.y));
}

TEST_CASE("panByScreenDelta shifts world origin", "[drl][GameCamera]")
{
	GameCamera camera;
	camera.panByScreenDelta({ 10.0f, -4.0f });
	const glm::vec2 world = camera.screenToWorld({ 0.0f, 0.0f });
	REQUIRE(world.x == Catch::Approx(-10.0f));
	REQUIRE(world.y == Catch::Approx(4.0f));
}

TEST_CASE("zoomAt keeps the cursor world point fixed", "[drl][GameCamera]")
{
	GameCamera camera;
	const glm::vec2 cursor{ 320.0f, 200.0f };
	const glm::vec2 before = camera.screenToWorld(cursor);
	camera.zoomAt(cursor, GameCamera::ZoomStep);
	const glm::vec2 after = camera.screenToWorld(cursor);
	REQUIRE(after.x == Catch::Approx(before.x));
	REQUIRE(after.y == Catch::Approx(before.y));
	REQUIRE(camera.zoom() == Catch::Approx(GameCamera::ZoomStep));
}

TEST_CASE("zoomAt clamps to ZoomMin and ZoomMax", "[drl][GameCamera]")
{
	GameCamera camera;
	const glm::vec2 cursor{ 100.0f, 100.0f };
	for (int i = 0; i < 64; ++i)
	{
		camera.zoomAt(cursor, GameCamera::ZoomStep);
	}
	REQUIRE(camera.zoom() == Catch::Approx(GameCamera::ZoomMax));

	for (int i = 0; i < 128; ++i)
	{
		camera.zoomAt(cursor, 1.0f / GameCamera::ZoomStep);
	}
	REQUIRE(camera.zoom() == Catch::Approx(GameCamera::ZoomMin));
}

}
}
