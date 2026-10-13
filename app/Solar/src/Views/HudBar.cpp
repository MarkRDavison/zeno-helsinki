#include <Views/HudBar.hpp>
#include <World/GalaxyDate.hpp>
#include <imgui.h>
#include <string>

namespace slr
{

	HudBarResult HudBar::draw(const SimClock& clock, long long tick, const WorldData& world) const
	{
		HudBarResult hud{};
		const std::string date = formatGalaxyDate(tick);
		const float padding = 12.0f;
		const float gap = 6.0f;
		const float rowHeight = 24.0f;
		const float policiesWidth = 88.0f;
		const float designWidth = 88.0f;
		const float researchWidth = 88.0f;
		const float timeWidth = ImGui::CalcTextSize(date.c_str()).x + gap +
			TimeChrome::SpeedButtonWidth * 4.0f + gap * 3.0f;

		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, BarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin(
			"HudBar",
			nullptr,
			ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoNav);

		ImGui::SetCursorPosX(padding);
		ImGui::SetCursorPosY(8.0f);
		const float situationWidth =
			ImGui::GetWindowSize().x - padding * 2.0f - gap * 4.0f - policiesWidth - designWidth -
			researchWidth - timeWidth;
		ImGui::BeginChild(
			"SituationChrome",
			ImVec2(situationWidth, rowHeight),
			false,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
		_situationChrome.draw(world);
		ImGui::EndChild();

		ImGui::SameLine(0.0f, gap);
		if (ImGui::Button("Policies", ImVec2(policiesWidth, rowHeight)))
		{
			hud.openPolicyEditor = true;
		}

		ImGui::SameLine(0.0f, gap);
		if (ImGui::Button("Design", ImVec2(designWidth, rowHeight)))
		{
			hud.openDesignEditor = true;
		}

		ImGui::SameLine(0.0f, gap);
		if (ImGui::Button("Research", ImVec2(researchWidth, rowHeight)))
		{
			hud.openResearchEditor = true;
		}

		ImGui::SameLine(0.0f, gap);
		ImGui::BeginChild(
			"TimeChrome",
			ImVec2(timeWidth, rowHeight),
			false,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
		hud.time = _timeChrome.draw(clock, tick);
		ImGui::EndChild();

		ImGui::End();
		ImGui::PopStyleVar(2);
		return hud;
	}

}
