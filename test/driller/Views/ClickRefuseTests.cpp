#include <catch2/catch_test_macros.hpp>
#include <Views/ClickRefuse.hpp>
#include <Views/WorldPopup.hpp>

namespace drl
{
namespace ClickRefuseTests
{

	TEST_CASE("next shaft that is affordable is not a refuse", "[drl][ClickRefuse]")
	{
		const ClickRefuse refuse = classifyShaftClick({ 0, 1 }, 0, true);
		REQUIRE(refuse.kind == ClickRefuseKind::None);
		REQUIRE(refuse.tile == glm::ivec2(0, 1));
	}

	TEST_CASE("next shaft that is broke flashes, no copy", "[drl][ClickRefuse]")
	{
		const ClickRefuse refuse = classifyShaftClick({ 0, 1 }, 0, false);
		REQUIRE(refuse.kind == ClickRefuseKind::ShaftBroke);
		REQUIRE(refuse.message.empty());
		REQUIRE(refuse.tile == glm::ivec2(0, 1));
	}

	TEST_CASE("shaft click not on the next level is quiet", "[drl][ClickRefuse]")
	{
		const ClickRefuse refuse = classifyShaftClick({ 0, 0 }, 0, true);
		REQUIRE(refuse.kind == ClickRefuseKind::None);
		REQUIRE(refuse.message.empty());
		REQUIRE(classifyShaftClick({ 0, 2 }, 0, false).kind == ClickRefuseKind::None);
	}

	TEST_CASE("place refuse broke vs invalid vs success", "[drl][ClickRefuse]")
	{
		REQUIRE(classifyPlaceRefuse({ 2, 1 }, true, true).kind == ClickRefuseKind::None);

		const ClickRefuse broke = classifyPlaceRefuse({ 2, 1 }, false, false);
		REQUIRE(broke.kind == ClickRefuseKind::PlaceBroke);
		REQUIRE(broke.tile == glm::ivec2(2, 1));
		REQUIRE(formatCantAffordPlace(50, "Money") == "Can't afford 50 Money");

		const ClickRefuse invalid = classifyPlaceRefuse({ -1, 0 }, true, false);
		REQUIRE(invalid.kind == ClickRefuseKind::PlaceInvalid);
		REQUIRE(invalid.message == kClickRefusePlaceInvalid);
		REQUIRE(invalid.tile == glm::ivec2(-1, 0));
	}

	TEST_CASE("world popup anchors the tile and rises in world space", "[drl][ClickRefuse]")
	{
		const glm::vec2 center = worldPopupTileCenter({ 2, 1 }, 0.0f, 64.0f, 64.0f);
		REQUIRE(center.x == 2.5f * 64.0f);
		REQUIRE(center.y == 64.0f + 1.5f * 64.0f);

		const glm::vec3 start = worldPopupWorldPosition({ 2, 1 }, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f);
		REQUIRE(start.x == center.x);
		REQUIRE(start.y == center.y);

		const glm::vec3 later = worldPopupWorldPosition({ 2, 1 }, 0.0f, 64.0f, 64.0f, 0.5f, 1.0f);
		REQUIRE(later.x == start.x);
		REQUIRE(later.y == start.y - 0.5f * kWorldPopupRisePixels);

		const glm::vec3 zoomed = worldPopupWorldPosition({ 2, 1 }, 0.0f, 64.0f, 64.0f, 0.5f, 2.0f);
		REQUIRE(zoomed.y == center.y - 0.5f * kWorldPopupRisePixels / 2.0f);
		REQUIRE(worldPopupScale(2.0f).x == 0.5f);
		REQUIRE(worldPopupColour(0.0f).a == 1.0f);
		REQUIRE(worldPopupColour(kWorldPopupDuration).a == 0.0f);
	}

}
}
