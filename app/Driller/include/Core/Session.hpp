#pragma once

#include <Entities/Data/GameData.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <string>

namespace drl
{

	class Session
	{
	public:
		Session();

		void loadGameSettings(const std::string& gameJsonPath);
		void loadAndValidate();

		bool skipToGameplay() const { return _skipToGameplay; }
		float simSpeed() const { return _simSpeed; }
		bool loadSucceeded() const { return _loadSucceeded; }

		GameData& gameData() { return _gameData; }
		const GameData& gameData() const { return _gameData; }
		TerrainAlterationService& terrainService() { return _terrainService; }
		const TerrainAlterationService& terrainService() const { return _terrainService; }

	private:
		bool _settingsLoaded{ false };
		bool _loadSucceeded{ false };
		bool _skipToGameplay{ false };
		float _simSpeed{ 1.0f };
		GameData _gameData;
		TerrainAlterationService _terrainService;
	};

}
