#include <Core/Session.hpp>
#include <Core/LoadError.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/NeedBindings.hpp>
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
		, _workerRecruitmentService(_gameData.worker, _workerPrototypeService)
		, _workerMovementService(_gameData.worker, _gameData.job, _terrainService, _gameData.shuttle)
		, _workerJobUpdateService(_gameData.worker, _gameData.job, _terrainService, _jobPrototypeService)
		, _workerNeedService(
			_gameData.worker,
			_needPrototypeService,
			_gameData.job,
			_jobPrototypeService)
		, _jobAllocationService(
			_gameData.job,
			_gameData.worker,
			_terrainService,
			_workerPrototypeService,
			_jobPrototypeService,
			_workerNeedService)
		, _workerCreationService(
			_gameData.worker,
			_workerPrototypeService,
			_gameData.building,
			_buildingPrototypeService,
			_needPrototypeService)
		, _buildingPlacementService(
			_gameData.building,
			_terrainService,
			_workerRecruitmentService,
			_jobCreationService,
			_buildingPrototypeService)
		, _shuttleCreationService(_gameData.shuttle, _shuttlePrototypeService)
		, _shuttleScheduleService(
			_gameData.shuttle,
			_gameData.worker,
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
			_upgradeService,
			_gameData.worker)
		, _game(_commandService, _simSpeed)
	{
		bindPrototypeUserTypes(_lua.raw());
		bindGameCommands(_lua.raw(), _commandService);
		_game.addTickService(_shuttleScheduleService);
		_game.addTickService(_workerMovementService);
		_game.addTickService(_workerJobUpdateService);
		_game.addTickService(_workerNeedService);
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

		_lua.runFile((scriptsDirectory / "needs.lua").string());
		applyNeedsTable(_lua.raw()["needs"], _needPrototypeService);

		_lua.runFile((scriptsDirectory / "upgrades.lua").string());
		applyUpgradesTable(_lua.raw()["upgrades"], _upgradeService);

		_lua.runFile((scriptsDirectory / "prototypes.lua").string());
		applyPrototypesTable(
			_lua.raw()["prototypes"],
			_jobPrototypeService,
			_workerPrototypeService,
			_buildingPrototypeService,
			_shuttlePrototypeService,
			_needPrototypeService);

		_lua.runFile((scriptsDirectory / "initializeCommands.lua").string());

		_loadSucceeded = true;
	}

}
