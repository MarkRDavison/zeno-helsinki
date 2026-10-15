#pragma once

#include <Views/MainView.hpp>
#include <Views/SystemMapCamera.hpp>
#include <World/GameCommand.hpp>
#include <World/WorldData.hpp>
#include <optional>
#include <vector>

namespace slr
{

	class GalaxyView;

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

		SystemMapCamera& camera() { return _camera; }
		const SystemMapCamera& camera() const { return _camera; }

		void draw(
			const WorldData& world,
			MainView& mainView,
			GalaxyView& galaxy,
			std::vector<GameCommand>& pendingCommands);

	private:
		std::optional<long long> _selectedPlaceId;
		SystemMapCamera _camera;
		std::optional<long long> _cameraSystemId;
	};

}
