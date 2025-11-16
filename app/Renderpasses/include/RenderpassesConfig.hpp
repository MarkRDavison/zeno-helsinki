#pragma once

#include <string>
#include <cstdint>
#include <fstream>
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
			const auto& doc = hl::Json::parseFromText(readFile(configPath));
			hl::JsonNode& root = *doc.m_Root;

			RenderpassConfig c;

			c.EnableVsync = root["EnableVsync"].boolean;
			c.DisplayFps = root["DisplayFps"].boolean;
			c.Title = root["Title"].content;
			c.Width = root["Width"].integer;
			c.Height = root["Height"].integer;

			return c;
		}

	private:

		static std::string readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::in | std::ios::binary);
			if (!file)
			{
				throw std::runtime_error("Failed to open file: " + filename);
			}

			std::ostringstream contents;
			contents << file.rdbuf();
			return contents.str();
		}
	};

}