#pragma once

#include <Services/CampaignTypes.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tower
{
	class CampaignCatalog
	{
	public:
		void load(const std::string& path);
		void loadFromText(const std::string& text, const char* file);
		const CampaignData& data() const;
		const CampaignNode* find(std::string_view id) const;
		const std::vector<CampaignNode>& nodes() const;

	private:
		CampaignData _data;
		std::unordered_map<std::string, std::size_t> _byId;
	};
}
