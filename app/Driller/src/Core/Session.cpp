#include <Core/Session.hpp>
#include <Core/LoadError.hpp>
#include <Core/SeedStartingCavern.hpp>
#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>

namespace drl
{

	Session::Session()
		: _terrainService(_gameData.terrain)
		, _commandService(_terrainService, _economyService)
	{
	}

	void Session::loadGameSettings(const std::string& gameJsonPath)
	{
		_skipToGameplay = false;
		_simSpeed = 1.0f;

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

		if (_gameData.terrain.shaftLevel < 0)
		{
			seedStartingCavern(_terrainService);
		}

		if (!_economyService.exists(ResourceMoney))
		{
			_economyService.setMax(ResourceOre, -1);
			_economyService.set(ResourceOre, 0);
			_economyService.setMax(ResourceMoney, -1);
			_economyService.set(ResourceMoney, 500);
		}

		_loadSucceeded = true;
	}

}
