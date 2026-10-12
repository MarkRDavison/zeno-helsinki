#include <Views/TimeChrome.hpp>
#include <World/GalaxyDate.hpp>
#include <imgui.h>
#include <string>

namespace slr
{

	TimeSpeedControl activeTimeControl(bool paused, int speed)
	{
		if (paused)
		{
			return TimeSpeedControl::Pause;
		}
		if (speed == 2)
		{
			return TimeSpeedControl::Speed2;
		}
		if (speed == 4)
		{
			return TimeSpeedControl::Speed4;
		}
		return TimeSpeedControl::Play;
	}

	namespace
	{
		const ImVec4 ActiveFill{ 1.0f, 0.85f, 0.2f, 1.0f };
		const ImVec4 ActiveFillHovered{ 1.0f, 0.90f, 0.35f, 1.0f };
		const ImVec4 ActiveText{ 0.12f, 0.10f, 0.04f, 1.0f };
		const ImVec4 InactiveFill{ 0.12f, 0.14f, 0.18f, 1.0f };
		const ImVec4 InactiveFillHovered{ 0.22f, 0.24f, 0.30f, 1.0f };
		const ImVec4 InactiveText{ 1.0f, 1.0f, 1.0f, 1.0f };

		bool speedButton(const char* label, bool active)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, active ? ActiveText : InactiveText);
			ImGui::PushStyleColor(ImGuiCol_Button, active ? ActiveFill : InactiveFill);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? ActiveFillHovered : InactiveFillHovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, active ? ActiveFill : InactiveFill);
			const bool clicked = ImGui::Button(label, ImVec2(TimeChrome::SpeedButtonWidth, 24.0f));
			ImGui::PopStyleColor(4);
			return clicked;
		}
	}

	TimeChromeResult TimeChrome::draw(const SimClock& clock, long long tick) const
	{
		TimeChromeResult result{};
		const auto active = activeTimeControl(clock.paused(), clock.speed());
		const std::string date = formatGalaxyDate(tick);
		const float gap = 6.0f;

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(date.c_str());
		ImGui::SameLine(0.0f, gap);

		if (speedButton("||", active == TimeSpeedControl::Pause) && !clock.paused())
		{
			result.paused = true;
		}
		ImGui::SameLine(0.0f, gap);
		if (speedButton(">", active == TimeSpeedControl::Play)
			&& (clock.paused() || clock.speed() != 1))
		{
			result.speed = 1;
			result.paused = false;
		}
		ImGui::SameLine(0.0f, gap);
		if (speedButton(">>", active == TimeSpeedControl::Speed2)
			&& (clock.paused() || clock.speed() != 2))
		{
			result.speed = 2;
			result.paused = false;
		}
		ImGui::SameLine(0.0f, gap);
		if (speedButton(">>>", active == TimeSpeedControl::Speed4)
			&& (clock.paused() || clock.speed() != 4))
		{
			result.speed = 4;
			result.paused = false;
		}

		return result;
	}

}
