#include <Services/UiService.hpp>

namespace drl
{

	UiService::UiService(const BuildingPrototypeService& buildingPrototypes)
		: _buildingPrototypes(buildingPrototypes)
	{
	}

	void UiService::update(const IUiInput& input)
	{
		if (!input.isKeyDown(kUiKeyEscape))
		{
			return;
		}

		if (!_activeBuilding.empty())
		{
			clearActiveBuilding();
		}

		if (_selectedWorkerId != 0)
		{
			clearSelectedWorker();
		}
	}

	void UiService::selectBuilding(const std::string& prototypeName)
	{
		for (const std::string& name : _buildingPrototypes.registeredNames())
		{
			if (name == prototypeName)
			{
				_activeBuilding = prototypeName;
				_currentState = UiState::PlacingBuilding;
				clearSelectedWorker();
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

	void UiService::selectWorker(WorkerId workerId)
	{
		_selectedWorkerId = workerId;
	}

	void UiService::clearSelectedWorker()
	{
		_selectedWorkerId = 0;
	}

	WorkerId UiService::selectedWorkerId() const
	{
		return _selectedWorkerId;
	}

}
