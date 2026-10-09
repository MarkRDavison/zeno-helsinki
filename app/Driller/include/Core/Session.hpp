#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/GameData.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobAllocationService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerMovementService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/Scripting/LuaState.hpp>
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
		EconomyResourceService& economyService() { return _economyService; }
		const EconomyResourceService& economyService() const { return _economyService; }
		JobPrototypeService& jobPrototypeService() { return _jobPrototypeService; }
		const JobPrototypeService& jobPrototypeService() const { return _jobPrototypeService; }
		JobCreationService& jobCreationService() { return _jobCreationService; }
		const JobCreationService& jobCreationService() const { return _jobCreationService; }
		JobAllocationService& jobAllocationService() { return _jobAllocationService; }
		const JobAllocationService& jobAllocationService() const { return _jobAllocationService; }
		WorkerPrototypeService& workerPrototypeService() { return _workerPrototypeService; }
		const WorkerPrototypeService& workerPrototypeService() const { return _workerPrototypeService; }
		WorkerCreationService& workerCreationService() { return _workerCreationService; }
		const WorkerCreationService& workerCreationService() const { return _workerCreationService; }
		WorkerMovementService& workerMovementService() { return _workerMovementService; }
		const WorkerMovementService& workerMovementService() const { return _workerMovementService; }
		GameCommandService& commandService() { return _commandService; }
		const GameCommandService& commandService() const { return _commandService; }
		Game& game() { return _game; }
		const Game& game() const { return _game; }
		hl::scripting::LuaState& lua() { return _lua; }
		const hl::scripting::LuaState& lua() const { return _lua; }

	private:
		bool _settingsLoaded{ false };
		bool _loadSucceeded{ false };
		bool _skipToGameplay{ false };
		float _simSpeed{ 1.0f };
		std::string _dataDirectory;
		hl::scripting::LuaState _lua;
		GameData _gameData;
		TerrainAlterationService _terrainService;
		EconomyResourceService _economyService;
		JobPrototypeService _jobPrototypeService;
		JobCreationService _jobCreationService;
		WorkerPrototypeService _workerPrototypeService;
		WorkerCreationService _workerCreationService;
		WorkerMovementService _workerMovementService;
		JobAllocationService _jobAllocationService;
		GameCommandService _commandService;
		Game _game;
	};

}
