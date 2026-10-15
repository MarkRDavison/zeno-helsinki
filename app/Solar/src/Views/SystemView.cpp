#include <Views/SystemView.hpp>
#include <Views/GalaxyView.hpp>
#include <Views/HudBar.hpp>
#include <Views/MoverDrawColours.hpp>
#include <Views/PlaceInspectChrome.hpp>
#include <Views/SystemMapMoverLayout.hpp>
#include <World/GameCommand.hpp>
#include <World/PickSystemMap.hpp>
#include <World/SystemPlaces.hpp>
#include <algorithm>
#include <imgui.h>
#include <string>

namespace slr
{
namespace
{

	constexpr float SystemMapYardMarkerHalf = 7.0f;
	constexpr float SystemMapJumpMarkerRadius = 6.0f;

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

	const char* truckLabel(const WorldData& world, const Truck& truck)
	{
		if (truckHasRole(world, truck, HullRole::Tender))
		{
			return "tender";
		}

		if (truckHasRole(world, truck, HullRole::MilitaryCargo))
		{
			return "military";
		}

		return "truck";
	}

	bool fleetMayReceiveMoveOrder(const WorldData& world, const Fleet& fleet)
	{
		if (fleet.escortConvoyId == 0)
		{
			return true;
		}

		return !fleetBlocksFleetOrganize(world, fleet);
	}

}

	void SystemView::applyMainView(const MainView& mainView)
	{
		if (!mainView.isSystem())
		{
			_selectedPlaceId.reset();
			_cameraSystemId.reset();
		}
	}

	void SystemView::draw(
		const WorldData& world,
		MainView& mainView,
		GalaxyView& galaxy,
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

		ImGui::SameLine();
		if (ImGui::Button("Reset view"))
		{
			_camera.resetToFit(*layout);
		}

		const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
		const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
		_camera.setCanvas(canvasMin, canvasSize);
		if (!_cameraSystemId.has_value() || *_cameraSystemId != systemId)
		{
			_cameraSystemId = systemId;
			_camera.resetToFit(*layout);
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const glm::vec2 starPos = starPositionInSystemSpace(*layout);
		const ImVec2 starScreen = _camera.systemSpaceToScreen(starPos);
		const bool starSelected =
			_selectedPlaceId.has_value() && starPlaceOf(*layout) != nullptr &&
			*_selectedPlaceId == starPlaceOf(*layout)->id;

		drawList->AddCircle(
			starScreen,
			outRadiusForLayout(*layout) * _camera.effectiveScale(),
			IM_COL32(90, 140, 190, 220),
			64,
			2.0f);
		drawList->AddCircleFilled(
			starScreen,
			layout->starSize * _camera.effectiveScale(),
			starFillColour(starSelected),
			32);
		drawPlaceLabel(drawList, starScreen, "Star");

		for (const auto& place : layout->places)
		{
			const bool selected = _selectedPlaceId.has_value() && *_selectedPlaceId == place.id;
			const ImVec2 placeScreen = _camera.systemSpaceToScreen(place.position);
			if (place.kind == SystemPlaceKind::Jump)
			{
				drawList->AddCircleFilled(
					placeScreen,
					SystemMapJumpMarkerRadius,
					jumpFillColour(selected),
					12);
				drawPlaceLabel(drawList, placeScreen, "Jump");
			}
			else if (place.kind == SystemPlaceKind::Yard)
			{
				drawList->AddRectFilled(
					ImVec2(placeScreen.x - SystemMapYardMarkerHalf, placeScreen.y - SystemMapYardMarkerHalf),
					ImVec2(placeScreen.x + SystemMapYardMarkerHalf, placeScreen.y + SystemMapYardMarkerHalf),
					yardFillColour(selected));
				drawPlaceLabel(drawList, placeScreen, "Yard");
			}
			else if (place.kind == SystemPlaceKind::Factory)
			{
				drawDiamondMarker(drawList, placeScreen, SystemMapYardMarkerHalf, factoryFillColour(selected));
				drawPlaceLabel(drawList, placeScreen, "Mill");
			}
			else if (place.kind == SystemPlaceKind::Mine)
			{
				drawList->AddCircleFilled(
					placeScreen,
					SystemMapJumpMarkerRadius,
					mineFillColour(selected),
					12);
				drawPlaceLabel(drawList, placeScreen, "Mine");
			}
		}

		const std::vector<SystemMapMoverDraw> movers = listMoversForSystem(world, systemId, *layout);
		for (const auto& mover : movers)
		{
			const ImVec2 moverScreen = _camera.systemSpaceToScreen(mover.systemPosition);
			if (mover.ref.kind == SystemMapMoverKind::Fleet)
			{
				const Fleet* fleet = world.fleetById(mover.ref.id);
				if (fleet == nullptr)
				{
					continue;
				}

				const bool selected =
					galaxy.selectedFleetId().has_value() && *galaxy.selectedFleetId() == fleet->id;
				drawFleetChevron(
					drawList,
					moverScreen,
					vec4ToImU32(fleetDrawColour(world, *fleet, selected)));
				drawPlaceLabel(drawList, moverScreen, fleet->name.c_str());
			}
			else
			{
				const Truck* truck = world.truckById(mover.ref.id);
				if (truck == nullptr)
				{
					continue;
				}

				const bool selected =
					galaxy.selectedTruckId().has_value() && *galaxy.selectedTruckId() == truck->id;
				drawTruckPill(
					drawList,
					moverScreen,
					vec4ToImU32(truckDrawColour(world, *truck, selected)));
				drawPlaceLabel(drawList, moverScreen, truckLabel(world, *truck));
			}
		}

		ImGui::InvisibleButton("system_map_canvas", canvasSize);
		const bool canvasHovered = ImGui::IsItemHovered();
		if (canvasHovered)
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
			{
				_camera.panByScreenDelta(ImGui::GetIO().MouseDelta);
			}

			const float wheel = ImGui::GetIO().MouseWheel;
			if (wheel != 0.0f)
			{
				_camera.zoomAtScreen(
					ImGui::GetIO().MousePos,
					wheel > 0.0f ? 1 : -1);
			}
		}

		if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			const SystemMapPick pick = pickSystemMap(
				world,
				systemId,
				*layout,
				_camera,
				movers,
				ImGui::GetIO().MousePos);
			if (pick.kind == SystemMapPickKind::Fleet)
			{
				galaxy.setSelectedFleetId(pick.id);
				_selectedPlaceId.reset();
			}
			else if (pick.kind == SystemMapPickKind::Truck)
			{
				galaxy.setSelectedTruckId(pick.id);
				_selectedPlaceId.reset();
			}
			else if (pick.kind == SystemMapPickKind::Place)
			{
				_selectedPlaceId = pick.id;
				galaxy.setSelectedFleetId(std::nullopt);
				galaxy.setSelectedTruckId(std::nullopt);
			}
			else
			{
				_selectedPlaceId.reset();
				galaxy.setSelectedFleetId(std::nullopt);
				galaxy.setSelectedTruckId(std::nullopt);
			}
		}

		if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			const auto fleetId = galaxy.selectedFleetId();
			if (fleetId.has_value())
			{
				const Fleet* fleet = world.fleetById(*fleetId);
				const SystemMapPick pick = pickSystemMap(
					world,
					systemId,
					*layout,
					_camera,
					movers,
					ImGui::GetIO().MousePos);
				if (fleet != nullptr && isPlayerFleet(world, *fleet) &&
					pick.kind == SystemMapPickKind::Place && fleetMayReceiveMoveOrder(world, *fleet))
				{
					pendingCommands.push_back(GameCommand::moveFleetToPlace(
						*fleetId,
						pick.id,
						CommandSource::Player));
				}
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
