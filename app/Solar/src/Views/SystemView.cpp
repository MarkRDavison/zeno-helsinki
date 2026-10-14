#include <Views/SystemView.hpp>
#include <Views/HudBar.hpp>
#include <Views/PlaceInspectChrome.hpp>
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
	constexpr float SystemMapYardMarkerHalf = 7.0f;
	constexpr float SystemMapJumpMarkerRadius = 6.0f;
	constexpr float SystemMapMoverOffsetX = 16.0f;
	constexpr float SystemMapMoverSlotY = 14.0f;

	ImVec2 systemSpaceToScreen(const ImVec2& origin, float scale, glm::vec2 systemPos)
	{
		return ImVec2(origin.x + systemPos.x * scale, origin.y - systemPos.y * scale);
	}

	glm::vec2 screenToSystemSpace(const ImVec2& origin, float scale, const ImVec2& screen)
	{
		if (scale <= 0.0f)
		{
			return glm::vec2{ 0.0f, 0.0f };
		}

		return glm::vec2((screen.x - origin.x) / scale, -(screen.y - origin.y) / scale);
	}

	void drawPlaceLabel(ImDrawList* drawList, const ImVec2& screen, const char* text)
	{
		drawList->AddText(ImVec2(screen.x + 8.0f, screen.y - 8.0f), IM_COL32(230, 230, 235, 255), text);
	}

	ImU32 starFillColour(bool selected)
	{
		return selected ? IM_COL32(255, 235, 140, 255) : IM_COL32(255, 210, 80, 255);
	}

	ImU32 jumpFillColour(bool selected)
	{
		return selected ? IM_COL32(230, 245, 255, 255) : IM_COL32(180, 210, 255, 255);
	}

	ImU32 yardFillColour(bool selected)
	{
		return selected ? IM_COL32(200, 240, 150, 255) : IM_COL32(160, 200, 120, 255);
	}

	ImU32 factoryFillColour(bool selected)
	{
		return selected ? IM_COL32(255, 220, 160, 255) : IM_COL32(220, 180, 120, 255);
	}

	ImU32 mineFillColour(bool selected)
	{
		return selected ? IM_COL32(190, 210, 255, 255) : IM_COL32(150, 170, 220, 255);
	}

	void drawDiamondMarker(
		ImDrawList* drawList,
		const ImVec2& center,
		float halfExtent,
		ImU32 fillColour)
	{
		const ImVec2 points[4] = {
			ImVec2(center.x, center.y - halfExtent),
			ImVec2(center.x + halfExtent, center.y),
			ImVec2(center.x, center.y + halfExtent),
			ImVec2(center.x - halfExtent, center.y) };
		drawList->AddConvexPolyFilled(points, 4, fillColour);
	}

	void drawFleetChevron(ImDrawList* drawList, const ImVec2& tip, ImU32 colour)
	{
		const ImVec2 points[3] = {
			ImVec2(tip.x, tip.y - 8.0f),
			ImVec2(tip.x + 6.0f, tip.y + 6.0f),
			ImVec2(tip.x - 6.0f, tip.y + 6.0f) };
		drawList->AddConvexPolyFilled(points, 3, colour);
	}

	void drawTruckPill(ImDrawList* drawList, const ImVec2& center, ImU32 colour)
	{
		drawList->AddRectFilled(
			ImVec2(center.x - 8.0f, center.y - 4.0f),
			ImVec2(center.x + 8.0f, center.y + 4.0f),
			colour,
			4.0f);
	}

	ImVec2 moverScreenAtPlace(const ImVec2& placeScreen, int slot)
	{
		return ImVec2(
			placeScreen.x + SystemMapMoverOffsetX,
			placeScreen.y - SystemMapMoverSlotY * static_cast<float>(slot + 1));
	}

	const char* inspectorTitleForPlace(SystemPlaceKind kind)
	{
		switch (kind)
		{
		case SystemPlaceKind::Star:
			return "Star";
		case SystemPlaceKind::Jump:
			return "Jump";
		case SystemPlaceKind::Yard:
			return "Yard";
		case SystemPlaceKind::Factory:
			return "Factory";
		case SystemPlaceKind::Mine:
			return "Mine";
		}

		return "Place";
	}

}

	void SystemView::applyMainView(const MainView& mainView)
	{
		if (!mainView.isSystem())
		{
			_selectedPlaceId.reset();
		}
	}

	void SystemView::draw(
		const WorldData& world,
		MainView& mainView,
		std::vector<GameCommand>& pendingCommands)
	{
		applyMainView(mainView);
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
			_selectedPlaceId.reset();
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
		const bool starSelected =
			_selectedPlaceId.has_value() && starPlaceOf(*layout) != nullptr &&
			*_selectedPlaceId == starPlaceOf(*layout)->id;

		drawList->AddCircle(
			starScreen,
			outRadiusForLayout(*layout) * scale,
			IM_COL32(90, 140, 190, 220),
			64,
			2.0f);
		drawList->AddCircleFilled(
			starScreen,
			layout->starSize * scale,
			starFillColour(starSelected),
			32);
		drawPlaceLabel(drawList, starScreen, "Star");

		for (const auto& place : layout->places)
		{
			const bool selected = _selectedPlaceId.has_value() && *_selectedPlaceId == place.id;
			if (place.kind == SystemPlaceKind::Jump)
			{
				const ImVec2 jumpScreen = systemSpaceToScreen(origin, scale, place.position);
				drawList->AddCircleFilled(jumpScreen, SystemMapJumpMarkerRadius, jumpFillColour(selected), 12);
				drawPlaceLabel(drawList, jumpScreen, "Jump");
			}
			else if (place.kind == SystemPlaceKind::Yard)
			{
				const ImVec2 yardScreen = systemSpaceToScreen(origin, scale, place.position);
				drawList->AddRectFilled(
					ImVec2(yardScreen.x - SystemMapYardMarkerHalf, yardScreen.y - SystemMapYardMarkerHalf),
					ImVec2(yardScreen.x + SystemMapYardMarkerHalf, yardScreen.y + SystemMapYardMarkerHalf),
					yardFillColour(selected));
				drawPlaceLabel(drawList, yardScreen, "Yard");
			}
			else if (place.kind == SystemPlaceKind::Factory)
			{
				const ImVec2 factoryScreen = systemSpaceToScreen(origin, scale, place.position);
				drawDiamondMarker(
					drawList,
					factoryScreen,
					SystemMapYardMarkerHalf,
					factoryFillColour(selected));
				drawPlaceLabel(drawList, factoryScreen, "Mill");
			}
			else if (place.kind == SystemPlaceKind::Mine)
			{
				const ImVec2 mineScreen = systemSpaceToScreen(origin, scale, place.position);
				drawList->AddCircleFilled(
					mineScreen,
					SystemMapJumpMarkerRadius,
					mineFillColour(selected),
					12);
				drawPlaceLabel(drawList, mineScreen, "Mine");
			}
		}

		for (const auto& place : layout->places)
		{
			int moverSlot = 0;
			const ImVec2 placeScreen = systemSpaceToScreen(origin, scale, place.position);
			for (const auto& fleet : world.fleets)
			{
				if (fleet.systemId != systemId || !fleetIsParked(fleet) ||
					inSystemMotionIsCrawling(fleet.inSystem) || fleet.placeId != place.id)
				{
					continue;
				}

				const ImVec2 moverScreen = moverScreenAtPlace(placeScreen, moverSlot++);
				drawFleetChevron(drawList, moverScreen, IM_COL32(240, 90, 90, 255));
				drawPlaceLabel(drawList, moverScreen, fleet.name.c_str());
			}

			for (const auto& truck : world.trucks)
			{
				if (!isTruckAtSystem(truck, systemId) || inSystemMotionIsCrawling(truck.inSystem) ||
					truck.placeId != place.id)
				{
					continue;
				}

				const ImVec2 moverScreen = moverScreenAtPlace(placeScreen, moverSlot++);
				drawTruckPill(drawList, moverScreen, IM_COL32(240, 165, 55, 255));
				drawPlaceLabel(drawList, moverScreen, "truck");
			}
		}

		for (const auto& fleet : world.fleets)
		{
			if (fleet.systemId != systemId || !inSystemMotionIsCrawling(fleet.inSystem))
			{
				continue;
			}

			const auto pos = inSystemMoverPosition(world, systemId, fleet.placeId, fleet.inSystem);
			if (!pos.has_value())
			{
				continue;
			}

			const ImVec2 moverScreen = systemSpaceToScreen(origin, scale, *pos);
			drawFleetChevron(drawList, moverScreen, IM_COL32(240, 90, 90, 255));
			drawPlaceLabel(drawList, moverScreen, fleet.name.c_str());
		}

		for (const auto& truck : world.trucks)
		{
			if (!isTruckAtSystem(truck, systemId) || !inSystemMotionIsCrawling(truck.inSystem))
			{
				continue;
			}

			const auto pos = inSystemMoverPosition(world, systemId, truck.placeId, truck.inSystem);
			if (!pos.has_value())
			{
				continue;
			}

			const ImVec2 moverScreen = systemSpaceToScreen(origin, scale, *pos);
			drawTruckPill(drawList, moverScreen, IM_COL32(240, 165, 55, 255));
			drawPlaceLabel(drawList, moverScreen, "truck");
		}

		ImGui::InvisibleButton("system_map_canvas", canvasSize);
		if (ImGui::IsItemClicked())
		{
			const glm::vec2 systemPos = screenToSystemSpace(origin, scale, ImGui::GetIO().MousePos);
			const SystemPlace* picked = pickSystemPlace(*layout, systemPos);
			if (picked != nullptr)
			{
				_selectedPlaceId = picked->id;
			}
			else
			{
				_selectedPlaceId.reset();
			}
		}

		ImGui::End();
		ImGui::PopStyleVar(2);

		if (!_selectedPlaceId.has_value())
		{
			return;
		}

		const SystemPlace* selected = placeById(*layout, *_selectedPlaceId);
		if (selected == nullptr)
		{
			_selectedPlaceId.reset();
			return;
		}

		bool inspectorOpen = true;
		ImGui::SetNextWindowPos(ImVec2(24.0f, 56.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(inspectorTitleForPlace(selected->kind), &inspectorOpen))
		{
			if (selected->kind == SystemPlaceKind::Star)
			{
				drawStarPlaceInspector(*layout, *selected);
			}
			else if (selected->kind == SystemPlaceKind::Jump)
			{
				drawJumpPlaceInspector(world, *selected);
			}
			else if (selected->kind == SystemPlaceKind::Yard)
			{
				drawYardPlaceInspector(world, systemId, pendingCommands);
			}
			else if (selected->kind == SystemPlaceKind::Factory)
			{
				drawFactoryPlaceInspector(world, systemId);
			}
			else if (selected->kind == SystemPlaceKind::Mine)
			{
				drawMinePlaceInspector(world, systemId);
			}
		}
		ImGui::End();
		if (!inspectorOpen)
		{
			_selectedPlaceId.reset();
		}
	}

}
