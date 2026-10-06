#pragma once

#include <Services/CampaignTypes.hpp>
#include <optional>
#include <string>
#include <vector>

namespace tower
{
	struct CampaignProfile
	{
		CampaignProgress progress;
		int points = 0;
		// TODO: long term we will unlock research, this will have side effects including unlocking towers
		std::vector<std::string> ownedTowers{ "single" };
		int startingGoldRank = 0;
		int fireRateRank = 0;
	};

	inline constexpr int kFirstClearPoints = 100;

	std::optional<std::string> campaignSavePath();

	class ProfileService
	{
	public:
		void load(const std::string& path, bool resetOnBoot);
		void save() const;
		CampaignProgress& progress();
		const CampaignProgress& progress() const;
		const CampaignProfile& profile() const;
		void recordWin(const std::string& nodeId, const CampaignData& data);

	private:
		void applyDefaults();
		bool tryParse(const std::string& text);
		bool writeFile() const;

		CampaignProfile _profile;
		std::string _path;
		bool _canWrite = false;
	};
}
