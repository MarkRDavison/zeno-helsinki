#include <Views/SituationChrome.hpp>
#include <World/Goods.hpp>
#include <imgui.h>
#include <string>

namespace slr
{

	void SituationChrome::draw(const WorldData& world) const
	{
		const std::string steelTotal = std::string(goodName(SteelGoodId)) + " " +
			std::to_string(locatedGoodAmount(world, SteelGoodId));
		const std::string oreTotal = std::string(goodName(IronOreGoodId)) + " " +
			std::to_string(locatedGoodAmount(world, IronOreGoodId));

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(steelTotal.c_str());
		ImGui::SameLine(0.0f, 12.0f);
		ImGui::TextUnformatted(oreTotal.c_str());
	}

}
