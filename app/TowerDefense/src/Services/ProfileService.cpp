#include <Services/ProfileService.hpp>
#include <Services/CampaignEvaluator.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/GraphEvaluator.hpp>
#include <algorithm>
#include <ranges>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace tower
{
	namespace
	{
		std::optional<std::string> envPath(const char* name)
		{
#if defined(_WIN32)
			char* buffer = nullptr;
			std::size_t length = 0;
			if (_dupenv_s(&buffer, &length, name) != 0 || buffer == nullptr || buffer[0] == '\0')
			{
				std::free(buffer);
				return std::nullopt;
			}

			std::string value(buffer);
			std::free(buffer);
			return value;
#else
			const char* value = std::getenv(name);
			if (value == nullptr || value[0] == '\0')
			{
				return std::nullopt;
			}

			return std::string(value);
#endif
		}

		std::string jsonEscape(const std::string& value)
		{
			return hl::String::FixEscapedChars(value);
		}

		CampaignProfile defaultProfile()
		{
			CampaignProfile profile;
			profile.researched = { "single" };
			profile.ownedTowers = { "single" };
			return profile;
		}

		void deriveOwnedAndRanks(CampaignProfile& profile, const ResearchData& data)
		{
			profile.ownedTowers.clear();
			profile.startingGoldRank = 0;
			profile.fireRateRank = 0;
			for (const auto& node : data.nodes)
			{
				if (!profile.researched.contains(node.id))
				{
					continue;
				}

				if (node.effect.tower.has_value())
				{
					const auto& id = *node.effect.tower;
					if (std::ranges::find(profile.ownedTowers, id) == profile.ownedTowers.end())
					{
						profile.ownedTowers.push_back(id);
					}
				}

				if (node.effect.startingGoldRank.has_value())
				{
					profile.startingGoldRank = std::max(
						profile.startingGoldRank,
						*node.effect.startingGoldRank);
				}

				if (node.effect.fireRateRank.has_value())
				{
					profile.fireRateRank = std::max(
						profile.fireRateRank,
						*node.effect.fireRateRank);
				}
			}

			if (profile.ownedTowers.empty())
			{
				profile.ownedTowers.push_back("single");
			}
		}
	}

	std::optional<std::string> campaignSavePath()
	{
		std::filesystem::path root;
#if defined(_WIN32)
		const auto appData = envPath("APPDATA");
		if (!appData.has_value())
		{
			return std::nullopt;
		}

		root = std::filesystem::path(*appData);
#elif defined(__APPLE__)
		const auto home = envPath("HOME");
		if (!home.has_value())
		{
			return std::nullopt;
		}

		root = std::filesystem::path(*home) / "Library" / "Application Support";
#else
		if (const auto xdg = envPath("XDG_DATA_HOME"); xdg.has_value())
		{
			root = std::filesystem::path(*xdg);
		}
		else
		{
			const auto home = envPath("HOME");
			if (!home.has_value())
			{
				return std::nullopt;
			}

			root = std::filesystem::path(*home) / ".local" / "share";
		}
#endif

		const auto dir = root / "HelsinkiTowerDefense" / "Saves";
		std::error_code error;
		std::filesystem::create_directories(dir, error);
		if (error)
		{
			return std::nullopt;
		}

		return (dir / "save.json").string();
	}

	void ProfileService::applyDefaults()
	{
		_profile = defaultProfile();
	}

	void ProfileService::load(const std::string& path, bool resetOnBoot)
	{
		_path = path;
		_canWrite = !path.empty();
		applyDefaults();

		if (!_canWrite)
		{
			return;
		}

		std::error_code error;
		if (resetOnBoot)
		{
			std::filesystem::remove(_path, error);
		}

		if (!std::filesystem::exists(_path, error) || error)
		{
			writeFile();
			return;
		}

		try
		{
			const auto text = hl::String::readFile(_path);
			if (!tryParse(text))
			{
				applyDefaults();
			}
		}
		catch (...)
		{
			applyDefaults();
		}
	}

	void ProfileService::save() const
	{
		if (!_canWrite)
		{
			return;
		}

		writeFile();
	}

	CampaignProgress& ProfileService::progress()
	{
		return _profile.progress;
	}

	const CampaignProgress& ProfileService::progress() const
	{
		return _profile.progress;
	}

	const CampaignProfile& ProfileService::profile() const
	{
		return _profile;
	}

	void ProfileService::recordWin(const std::string& nodeId, const CampaignData& data)
	{
		if (nodeId.empty())
		{
			return;
		}

		if (!_profile.progress.cleared.contains(nodeId))
		{
			_profile.progress.cleared.insert(nodeId);
			_profile.points += kFirstClearPoints;
		}

		const auto states = evaluateCampaign(data, _profile.progress);
		_profile.progress.skipped.clear();
		for (const auto& [id, state] : states)
		{
			if (state == CampaignNodeState::Skipped)
			{
				_profile.progress.skipped.insert(id);
			}
		}

		save();
	}

	void ProfileService::applyResearchEffects(const ResearchData& data)
	{
		deriveOwnedAndRanks(_profile, data);
	}

	void ProfileService::syncFromResearch(const ResearchData& data)
	{
		for (const auto& node : data.nodes)
		{
			if (node.start)
			{
				_profile.researched.insert(node.id);
			}
		}

		applyResearchEffects(data);
	}

	bool ProfileService::tryResearch(const std::string& nodeId, const ResearchData& data)
	{
		if (nodeId.empty() || _profile.researched.contains(nodeId))
		{
			return false;
		}

		const ResearchNode* node = nullptr;
		for (const auto& candidate : data.nodes)
		{
			if (candidate.id == nodeId)
			{
				node = &candidate;
				break;
			}
		}

		if (node == nullptr)
		{
			return false;
		}

		const auto states = evaluateGraph(researchGraphNodes(data), _profile.researched);
		const auto it = states.find(nodeId);
		if (it == states.end() || it->second != GraphNodeState::Available)
		{
			return false;
		}

		if (_profile.points < node->cost)
		{
			return false;
		}

		_profile.points -= node->cost;
		_profile.researched.insert(nodeId);
		applyResearchEffects(data);
		save();
		return true;
	}

	bool ProfileService::tryParse(const std::string& text)
	{
		CampaignProfile parsed = defaultProfile();
		try
		{
			const auto doc = hl::Json::parseFromText(text);
			if (doc.m_Root == nullptr || doc.m_Root->type != hl::JsonNode::Type::Object)
			{
				return false;
			}

			const auto& root = *doc.m_Root;
		if (const auto* version = catalogJson::findChild(root, "version"); version != nullptr)
		{
			if (version->type != hl::JsonNode::Type::ValueInteger || version->integer < 1)
			{
				return false;
			}
		}

		if (const auto* cleared = catalogJson::findChild(root, "cleared"); cleared != nullptr)
		{
			if (cleared->type != hl::JsonNode::Type::Object)
			{
				return false;
			}

			parsed.progress.cleared.clear();
			for (const auto* child : cleared->children)
			{
				if (child == nullptr || child->name.empty())
				{
					return false;
				}

				parsed.progress.cleared.insert(child->name);
			}
		}

		if (const auto* skipped = catalogJson::findChild(root, "skipped"); skipped != nullptr)
		{
			if (skipped->type != hl::JsonNode::Type::Array)
			{
				return false;
			}

			parsed.progress.skipped.clear();
			for (const auto* child : skipped->children)
			{
				if (child == nullptr || child->type != hl::JsonNode::Type::ValueString || child->content.empty())
				{
					return false;
				}

				parsed.progress.skipped.insert(child->content);
			}
		}

		if (const auto* currency = catalogJson::findChild(root, "currency"); currency != nullptr)
		{
			if (currency->type != hl::JsonNode::Type::Object)
			{
				return false;
			}

			if (const auto* points = catalogJson::findChild(*currency, "points"); points != nullptr)
			{
				if (points->type != hl::JsonNode::Type::ValueInteger)
				{
					return false;
				}

				parsed.points = points->integer;
			}
		}

		if (const auto* unlocks = catalogJson::findChild(root, "unlocks"); unlocks != nullptr)
		{
			if (unlocks->type != hl::JsonNode::Type::Object)
			{
				return false;
			}

			if (const auto* towers = catalogJson::findChild(*unlocks, "towers"); towers != nullptr)
			{
				if (towers->type != hl::JsonNode::Type::Array)
				{
					return false;
				}

				parsed.ownedTowers.clear();
				for (const auto* child : towers->children)
				{
					if (child == nullptr || child->type != hl::JsonNode::Type::ValueString || child->content.empty())
					{
						return false;
					}

					parsed.ownedTowers.push_back(child->content);
				}

				if (parsed.ownedTowers.empty())
				{
					parsed.ownedTowers.push_back("single");
				}
			}
		}

		if (const auto* upgrades = catalogJson::findChild(root, "upgrades"); upgrades != nullptr)
		{
			if (upgrades->type != hl::JsonNode::Type::Object)
			{
				return false;
			}

			if (const auto* gold = catalogJson::findChild(*upgrades, "startingGold"); gold != nullptr)
			{
				if (gold->type != hl::JsonNode::Type::ValueInteger || gold->integer < 0)
				{
					return false;
				}

				parsed.startingGoldRank = gold->integer;
			}

			if (const auto* fire = catalogJson::findChild(*upgrades, "fireRate"); fire != nullptr)
			{
				if (fire->type != hl::JsonNode::Type::ValueInteger || fire->integer < 0)
				{
					return false;
				}

				parsed.fireRateRank = fire->integer;
			}
		}

		if (const auto* research = catalogJson::findChild(root, "research"); research != nullptr)
		{
			if (research->type != hl::JsonNode::Type::Array)
			{
				return false;
			}

			parsed.researched.clear();
			for (const auto* child : research->children)
			{
				if (child == nullptr || child->type != hl::JsonNode::Type::ValueString || child->content.empty())
				{
					return false;
				}

				parsed.researched.insert(child->content);
			}

			if (parsed.researched.empty())
			{
				parsed.researched.insert("single");
			}
		}

			_profile = std::move(parsed);
			return true;
		}
		catch (const std::string&)
		{
			return false;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	bool ProfileService::writeFile() const
	{
		if (_path.empty())
		{
			return false;
		}

		std::error_code error;
		std::filesystem::create_directories(std::filesystem::path(_path).parent_path(), error);
		if (error)
		{
			return false;
		}

		std::ostringstream out;
		out << "{\n";
		out << "  \"version\": 1,\n";
		out << "  \"cleared\": {";
		bool first = true;
		for (const auto& id : _profile.progress.cleared)
		{
			if (!first)
			{
				out << ",";
			}

			first = false;
			out << "\n    \"" << jsonEscape(id) << "\": {}";
		}

		if (!first)
		{
			out << "\n  ";
		}

		out << "},\n";
		out << "  \"skipped\": [";
		first = true;
		for (const auto& id : _profile.progress.skipped)
		{
			if (!first)
			{
				out << ", ";
			}

			first = false;
			out << "\"" << jsonEscape(id) << "\"";
		}

		out << "],\n";
		out << "  \"currency\": { \"points\": " << _profile.points << " },\n";
		out << "  \"research\": [";
		first = true;
		for (const auto& id : _profile.researched)
		{
			if (!first)
			{
				out << ", ";
			}

			first = false;
			out << "\"" << jsonEscape(id) << "\"";
		}

		out << "],\n";
		out << "  \"unlocks\": { \"towers\": [";
		first = true;
		for (const auto& id : _profile.ownedTowers)
		{
			if (!first)
			{
				out << ", ";
			}

			first = false;
			out << "\"" << jsonEscape(id) << "\"";
		}

		out << "] },\n";
		out << "  \"upgrades\": { \"startingGold\": " << _profile.startingGoldRank
			<< ", \"fireRate\": " << _profile.fireRateRank << " }\n";
		out << "}\n";

		std::ofstream file(_path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file)
		{
			return false;
		}

		file << out.str();
		return static_cast<bool>(file);
	}
}
