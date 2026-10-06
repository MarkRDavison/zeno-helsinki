#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>

#include <cmath>
#include <vector>

namespace
{
	constexpr float OrbitDegreesPerPixel = 0.25f;
	constexpr float Pitch = -45.0f;
	constexpr glm::vec3 Origin{ 0.0f, 0.0f, 0.0f };
	constexpr glm::vec3 WorldUp{ 0.0f, 1.0f, 0.0f };

	float orbitYaw(float startYaw, float startMouseX, float mouseX)
	{
		return startYaw - (mouseX - startMouseX) * OrbitDegreesPerPixel;
	}
}

TEST_CASE("setLookAtPose looks at target from spherical yaw/pitch", "[Camera][Orbit]")
{
	const float distance = glm::length(glm::vec3(0.0f, 16.0f, 16.0f));
	hl::Camera camera(glm::vec3(0.0f, 16.0f, 16.0f), WorldUp, -90.0f, Pitch);
	camera.setLookAtPose(Origin, -90.0f, Pitch, distance);

	CHECK(glm::length(camera.getPosition() - glm::vec3(0.0f, 16.0f, 16.0f)) < 1e-4f);
	CHECK(glm::length(camera.getFront() - glm::normalize(-camera.getPosition())) < 1e-4f);
}

TEST_CASE("absolute drag yaw is linear in mouse X", "[Camera][Orbit]")
{
	const float startYaw = -90.0f;
	CHECK(orbitYaw(startYaw, 100.0f, 140.0f) == Catch::Approx(-100.0f));
	CHECK(orbitYaw(startYaw, 100.0f, 100.4f) == Catch::Approx(startYaw - 0.1f));
}

TEST_CASE("absolute pose is stable across extra presents with the same mouse", "[Camera][Orbit]")
{
	const float distance = glm::length(glm::vec3(0.0f, 16.0f, 16.0f));
	hl::Camera camera(glm::vec3(0.0f, 16.0f, 16.0f), WorldUp, -90.0f, Pitch);
	const float startYaw = camera.getYaw();
	const float startMouse = 10.0f;
	const float mouse = 25.0f;
	const float yaw = orbitYaw(startYaw, startMouse, mouse);

	std::vector<glm::vec3> poses;
	for (int present = 0; present < 5; ++present)
	{
		camera.setLookAtPose(Origin, yaw, Pitch, distance);
		poses.push_back(camera.getPosition());
	}

	for (int i = 1; i < static_cast<int>(poses.size()); ++i)
	{
		CHECK(glm::length(poses[static_cast<size_t>(i)] - poses[0]) < 1e-6f);
	}
}

TEST_CASE("0.4px mouse steps still rotate with absolute mapping", "[Camera][Orbit]")
{
	const float distance = glm::length(glm::vec3(0.0f, 16.0f, 16.0f));
	hl::Camera camera(glm::vec3(0.0f, 16.0f, 16.0f), WorldUp, -90.0f, Pitch);
	const float startYaw = camera.getYaw();
	const glm::vec3 startPos = camera.getPosition();

	float mouse = 0.0f;
	for (int i = 0; i < 20; ++i)
	{
		mouse += 0.4f;
		camera.setLookAtPose(Origin, orbitYaw(startYaw, 0.0f, mouse), Pitch, distance);
	}

	CHECK(glm::length(camera.getPosition() - startPos) > 0.05f);
	CHECK(camera.getYaw() == Catch::Approx(startYaw - 8.0f * OrbitDegreesPerPixel));
}
