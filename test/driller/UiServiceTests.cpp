#include <catch2/catch_test_macros.hpp>
#include <Entities/Building.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/UiService.hpp>
#include <unordered_set>

namespace drl
{
namespace UiServiceTests
{

	struct FakeUiInput : IUiInput
	{
		std::unordered_set<int> down;

		bool isKeyDown(int key) const override
		{
			return down.contains(key);
		}
	};

	struct Fixture
	{
		BuildingPrototypeService buildingPrototypes;
		UiService ui{ buildingPrototypes };

		Fixture()
		{
			BuildingPrototype bunk{};
			bunk.name = "Building_Bunk";
			buildingPrototypes.registerPrototype(std::move(bunk));
			BuildingPrototype hut{};
			hut.name = "Building_Builders_Hut";
			buildingPrototypes.registerPrototype(std::move(hut));
		}
	};

	TEST_CASE("selectBuilding selects a registered prototype", "[drl][UiService]")
	{
		Fixture f;
		f.ui.selectBuilding("Building_Bunk");
		REQUIRE(f.ui.getCurrentState() == UiState::PlacingBuilding);
		REQUIRE(f.ui.getActiveBuildingType() == "Building_Bunk");
	}

	TEST_CASE("selectBuilding unknown name is a no-op", "[drl][UiService]")
	{
		Fixture f;
		f.ui.selectBuilding("Building_Missing");
		REQUIRE(f.ui.getCurrentState() == UiState::Default);
		REQUIRE(f.ui.getActiveBuildingType().empty());
	}

	TEST_CASE("escape clears active building", "[drl][UiService]")
	{
		Fixture f;
		f.ui.selectBuilding("Building_Bunk");
		REQUIRE(f.ui.getCurrentState() == UiState::PlacingBuilding);

		FakeUiInput escape;
		escape.down.insert(kUiKeyEscape);
		f.ui.update(escape);
		REQUIRE(f.ui.getCurrentState() == UiState::Default);
		REQUIRE(f.ui.getActiveBuildingType().empty());
	}

	TEST_CASE("clearActiveBuilding returns to default", "[drl][UiService]")
	{
		Fixture f;
		f.ui.selectBuilding("Building_Builders_Hut");
		f.ui.clearActiveBuilding();
		REQUIRE(f.ui.getCurrentState() == UiState::Default);
		REQUIRE(f.ui.getActiveBuildingType().empty());
	}

	TEST_CASE("digit keys do not select a building", "[drl][UiService]")
	{
		Fixture f;
		FakeUiInput input;
		input.down.insert(49);
		input.down.insert(50);
		f.ui.update(input);
		REQUIRE(f.ui.getCurrentState() == UiState::Default);
		REQUIRE(f.ui.getActiveBuildingType().empty());
	}

}
}
