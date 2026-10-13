#include <Views/SituationChrome.hpp>
#include <World/Goods.hpp>
#include <World/Research.hpp>
#include <imgui.h>
#include <string>

namespace slr
{
namespace
{

	std::string helionResearchSummary(const WorldData& world)
	{
		const long long activeCount = countActiveResearchProjects(world, HelionEmpireId);
		if (activeCount <= 0)
		{
			return "Research idle";
		}

		if (activeCount == 1)
		{
			for (const auto& row : world.researchProgress)
			{
				if (row.empireId != HelionEmpireId || !row.active || row.completed)
				{
					continue;
				}

				const ResearchProjectSpec* spec = researchProjectById(row.projectId);
				if (spec == nullptr)
				{
					continue;
				}

				return std::string(spec->name) + " " +
					std::to_string(researchDisplayedAmount(row.progress)) + " / " +
					std::to_string(researchDisplayedAmount(spec->cost));
			}
		}

		const long long usedPercent =
			researchDisplayedAmount(researchPoolUsedFraction(activeCount) * 100.f);
		return "Research " + std::to_string(activeCount) + " · " + std::to_string(usedPercent) + "%";
	}

}

	void SituationChrome::draw(const WorldData& world) const
	{
		const std::string steelTotal = std::string(goodName(SteelGoodId)) + " " +
			std::to_string(locatedGoodAmount(world, SteelGoodId));
		const std::string oreTotal = std::string(goodName(IronOreGoodId)) + " " +
			std::to_string(locatedGoodAmount(world, IronOreGoodId));
		const std::string researchTotal = helionResearchSummary(world);

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(steelTotal.c_str());
		ImGui::SameLine(0.0f, 12.0f);
		ImGui::TextUnformatted(oreTotal.c_str());
		ImGui::SameLine(0.0f, 12.0f);
		ImGui::TextUnformatted(researchTotal.c_str());
	}

}
