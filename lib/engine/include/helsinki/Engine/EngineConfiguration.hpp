#pragma once

#include <cstdint>
#include <string>
#include <format>
#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>

namespace hl
{
	struct EngineConfiguration
	{
		bool EnableVsync{ false };
		bool DisplayFps{ false };
		std::string Title{ "Helsinki Renderpasses" };
		std::string RootPath{ "" };
		uint32_t Width{ 800 };
		uint32_t Height{ 600 };
		uint32_t MaxMaterials{ 64 };

		void applyConfig(const std::string& filename, const std::string& rootPath)
		{
			const auto& parsed = EngineConfiguration::Load(std::format("{}{}", rootPath, filename));

			this->EnableVsync = parsed.EnableVsync;
			this->DisplayFps = parsed.DisplayFps;
			this->Title = parsed.Title;
			this->Width = parsed.Width;
			this->Height = parsed.Height;
			this->RootPath = rootPath;
		}

		static EngineConfiguration Load(const std::string& configPath)
		{
			const auto& doc = hl::Json::parseFromText(hl::String::readFile(configPath));
			hl::JsonNode& root = *doc.m_Root;

			EngineConfiguration c;

			c.EnableVsync = root["EnableVsync"].boolean;
			c.DisplayFps = root["DisplayFps"].boolean;
			c.Title = root["Title"].content;
			c.Width = root["Width"].integer;
			c.Height = root["Height"].integer;

			return c;
		}

	};
}