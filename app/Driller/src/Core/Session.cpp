#include <Core/Session.hpp>
#include <Core/LoadError.hpp>
#include <Scripting/CommandBindings.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Scripting/ResourceBindings.hpp>
#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <filesystem>

namespace drl
{

	Session::Session()
		: _terrainService(_gameData.terrain)
		, _jobCreationService(_gameData.job, _jobPrototypeService, _terrainService)
		, _workerCreationService(_gameData.worker, _workerPrototypeService)
		, _commandService(_terrainService, _economyService, _jobCreationService, _workerCreationService)
		, _game(_commandService, _simSpeed)
	{
		bindPrototypeUserTypes(_lua.raw());
		bindGameCommands(_lua.raw(), _commandService);
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

		_lua.runFile((scriptsDirectory / "prototypes.lua").string());
		applyPrototypesTable(_lua.raw()["prototypes"], _jobPrototypeService, _workerPrototypeService);

		_lua.runFile((scriptsDirectory / "initializeCommands.lua").string());

		_loadSucceeded = true;
	}

}
