#pragma once

#include <Views/MainView.hpp>
#include <World/GameCommand.hpp>
#include <World/WorldData.hpp>
#include <optional>
#include <vector>

namespace slr
{

	class SystemView
	{
	public:
		void applyMainView(const MainView& mainView);
		std::optional<long long> selectedPlaceId() const
		{
			return _selectedPlaceId;
		}

		void setSelectedPlaceId(std::optional<long long> placeId)
		{
			_selectedPlaceId = placeId;
		}

		void draw(const WorldData& world, MainView& mainView, std::vector<GameCommand>& pendingCommands);

	private:
		std::optional<long long> _selectedPlaceId;
	};

}
