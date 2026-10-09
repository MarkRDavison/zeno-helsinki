#include <Core/Session.hpp>
#include <Core/LoadError.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Scripting/ResourceBindings.hpp>
#include <Scripting/UpgradeBindings.hpp>
#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <filesystem>

namespace drl
{

	Session::Session()
		: _terrainService(_gameData.terrain)
		, _jobCreationService(_gameData.job, _jobPrototypeService, _terrainService)
		, _workerCreationService(_gameData.worker, _workerPrototypeService)
		, _workerRecruitmentService(_gameData.worker, _workerPrototypeService)
		, _workerMovementService(_gameData.worker, _gameData.job, _terrainService)
		, _workerJobUpdateService(_gameData.worker, _gameData.job, _terrainService, _jobPrototypeService)
		, _jobAllocationService(_gameData.job, _gameData.worker, _terrainService, _workerPrototypeService)
		, _buildingPlacementService(
			_gameData.building,
			_terrainService,
			_workerRecruitmentService,
			_jobCreationService,
			_buildingPrototypeService)
		, _shuttleCreationService(_gameData.shuttle, _shuttlePrototypeService)
		, _shuttleScheduleService(
			_gameData.shuttle,
			_workerRecruitmentService,
			_workerCreationService,
			_shuttlePrototypeService,
			_economyService)
		, _upgradeService(_gameData.upgrade)
		, _uiService(_buildingPrototypeService)
		, _commandService(
			_terrainService,
			_economyService,
			_jobCreationService,
			_workerCreationService,
			_buildingPlacementService,
			_buildingPrototypeService,
			_shuttleCreationService,
			_upgradeService)
		, _game(_commandService, _simSpeed)
	{
		bindPrototypeUserTypes(_lua.raw());
		bindGameCommands(_lua.raw(), _commandService);
		_game.addTickService(_shuttleScheduleService);
		_game.addTickService(_workerMovementService);
		_game.addTickService(_workerJobUpdateService);
		_game.addTickService(_jobAllocationService);
	}

	void Session::loadGameSettings(const std::string& gameJsonPath)
	{
		_skipToGameplay = false;
		_simSpeed = 1.0f;
		_dataDirectory = std::filesystem::path(gameJsonPath).parent_path().string();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(gameJsonPath));
		hl::JsonNode& root = *doc.m_Root;

		try
		{
			_skipToGameplay = root["SkipToGameplay"].boolean;
		}
		catch (const std::string&)
		{
		}

		try
		{
			const auto& node = root["SimSpeed"];
			if (node.type == hl::JsonNode::Type::ValueInteger)
			{
				_simSpeed = static_cast<float>(node.integer);
			}
			else
			{
				_simSpeed = node.number;
			}
		}
		catch (const std::string&)
		{
		}

		_settingsLoaded = true;
	}

	void Session::loadAndValidate()
	{
		if (_loadSucceeded)
		{
			return;
		}

		if (!_settingsLoaded)
		{
			throw LoadError("game.json settings were not loaded");
		}

		const auto scriptsDirectory = std::filesystem::path(_dataDirectory) / "Scripts" / "Base";
		_lua.runFile((scriptsDirectory / "resources.lua").string());
		applyResourcesTable(_lua.raw()["resources"], _economyService);

		_lua.runFile((scriptsDirectory / "upgrades.lua").string());
		applyUpgradesTable(_lua.raw()["upgrades"], _upgradeService);

		_lua.runFile((scriptsDirectory / "prototypes.lua").string());
		applyPrototypesTable(
			_lua.raw()["prototypes"],
			_jobPrototypeService,
			_workerPrototypeService,
			_buildingPrototypeService,
			_shuttlePrototypeService);

		_lua.runFile((scriptsDirectory / "initializeCommands.lua").string());

		_loadSucceeded = true;
	}

}
