#include <Services/UiService.hpp>

namespace drl
{

	UiService::UiService(const BuildingPrototypeService& buildingPrototypes)
		: _buildingPrototypes(buildingPrototypes)
	{
	}

	void UiService::update(const IUiInput& input)
	{
		if (input.isKeyDown(kUiKeyEscape) && !_activeBuilding.empty())
		{
			clearActiveBuilding();
			return;
		}

		const std::vector<std::string>& names = _buildingPrototypes.registeredNames();
		const int count = static_cast<int>(names.size()) < 9
			? static_cast<int>(names.size())
			: 9;
		for (int offset = 0; offset < count; ++offset)
		{
			if (input.isKeyDown(kUiKey1 + offset))
			{
				_currentState = UiState::PlacingBuilding;
				_activeBuilding = names[static_cast<unsigned>(offset)];
				return;
			}
		}
	}

	std::string UiService::getActiveBuildingType() const
	{
		return _activeBuilding;
	}

	UiState UiService::getCurrentState() const
	{
		return _currentState;
	}

	void UiService::clearActiveBuilding()
	{
		_activeBuilding.clear();
		_currentState = UiState::Default;
	}

}
