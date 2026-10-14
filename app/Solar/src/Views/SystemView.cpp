#include <Views/SystemView.hpp>
#include <Views/HudBar.hpp>
#include <World/SystemPlaces.hpp>
#include <algorithm>
#include <imgui.h>
#include <string>

namespace slr
{
namespace
{

	constexpr float SystemMapDefaultFitRadius = 120.0f;
	constexpr float SystemMapPad = 48.0f;

	ImVec2 systemSpaceToScreen(const ImVec2& origin, float scale, glm::vec2 systemPos)
	{
		return ImVec2(origin.x + systemPos.x * scale, origin.y - systemPos.y * scale);
	}

	void drawPlaceLabel(ImDrawList* drawList, const ImVec2& screen, const char* text)
	{
		drawList->AddText(ImVec2(screen.x + 8.0f, screen.y - 8.0f), IM_COL32(230, 230, 235, 255), text);
	}

}

	void SystemView::draw(const WorldData& world, MainView& mainView) const
	{
		if (!mainView.isSystem() || !mainView.openedSystemId().has_value())
		{
			return;
		}

		const long long systemId = *mainView.openedSystemId();
		const StarSystem* system = world.systemById(systemId);
		const char* systemName = system != nullptr ? system->name.c_str() : "Unknown";

		const ImVec2 display = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowPos(ImVec2(0.0f, HudBar::BarHeight));
		ImGui::SetNextWindowSize(ImVec2(display.x, display.y - HudBar::BarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin(
			"SystemMap",
			nullptr,
			ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoBringToFrontOnFocus);

		ImGui::TextUnformatted(systemName);
		ImGui::SameLine();
		if (ImGui::Button("Galaxy"))
		{
			mainView.returnToGalaxy();
			ImGui::End();
			ImGui::PopStyleVar(2);
			return;
		}

		const SystemLayout* layout = world.systemLayoutBySystemId(systemId);
		if (layout == nullptr)
		{
			ImGui::TextUnformatted("no places");
			ImGui::End();
			ImGui::PopStyleVar(2);
			return;
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
		const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
		const ImVec2 origin(
			canvasMin.x + canvasSize.x * 0.5f,
			canvasMin.y + canvasSize.y * 0.5f);
		const float fitRadius = (std::max)(outRadiusForLayout(*layout), SystemMapDefaultFitRadius);
		const float usable = (std::min)(canvasSize.x, canvasSize.y) - SystemMapPad * 2.0f;
		const float scale = usable > 0.0f ? usable / (fitRadius * 2.0f) : 1.0f;
		const glm::vec2 starPos = starPositionInSystemSpace(*layout);
		const ImVec2 starScreen = systemSpaceToScreen(origin, scale, starPos);

		drawList->AddCircle(
			starScreen,
			outRadiusForLayout(*layout) * scale,
			IM_COL32(90, 140, 190, 220),
			64,
			2.0f);
		drawList->AddCircleFilled(
			starScreen,
			layout->starSize * scale,
			IM_COL32(255, 210, 80, 255),
			32);
		drawPlaceLabel(drawList, starScreen, "Star");

		for (const auto& place : layout->places)
		{
			if (place.kind == SystemPlaceKind::Jump)
			{
				const ImVec2 jumpScreen = systemSpaceToScreen(origin, scale, place.position);
				drawList->AddCircleFilled(jumpScreen, 6.0f, IM_COL32(180, 210, 255, 255), 12);
				const StarSystem* neighbor = world.systemById(place.neighborSystemId);
				const char* jumpName = neighbor != nullptr ? neighbor->name.c_str() : "Jump";
				drawPlaceLabel(drawList, jumpScreen, jumpName);
			}
			else if (place.kind == SystemPlaceKind::Yard)
			{
				const ImVec2 yardScreen = systemSpaceToScreen(origin, scale, place.position);
				const float half = 7.0f;
				drawList->AddRectFilled(
					ImVec2(yardScreen.x - half, yardScreen.y - half),
					ImVec2(yardScreen.x + half, yardScreen.y + half),
					IM_COL32(160, 200, 120, 255));
				drawPlaceLabel(drawList, yardScreen, "Yard");
			}
		}

		ImGui::Dummy(canvasSize);
		ImGui::End();
		ImGui::PopStyleVar(2);
	}

}
