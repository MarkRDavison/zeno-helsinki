#pragma once

#include <string>
#include <cstdint>
#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>

namespace rp
{

	struct RenderpassConfig
	{
		bool EnableVsync{ false };
		bool DisplayFps{ false };
		std::string Title{ "Helsinki Renderpasses" };
		uint32_t Width{ 800 };
		uint32_t Height{ 600 };

		static RenderpassConfig Load(const std::string& configPath)
		{
			const auto& doc = hl::Json::parseFromText(hl::String::readFile(configPath));
			hl::JsonNode& root = *doc.m_Root;

			RenderpassConfig c;

			c.EnableVsync = root["EnableVsync"].boolean;
			c.DisplayFps = root["DisplayFps"].boolean;
			c.Title = root["Title"].content;
			c.Width = root["Width"].integer;
			c.Height = root["Height"].integer;

			return c;
		}
	};

}