#include <Views/DesignChrome.hpp>
#include <imgui.h>
#include <cstdio>
#include <string>

namespace slr
{
namespace
{

	HullSize hullSizeFromIndex(int index)
	{
		if (index == 1)
		{
			return HullSize::Medium;
		}

		if (index == 2)
		{
			return HullSize::Large;
		}

		return HullSize::Small;
	}

	int hullSizeIndex(HullSize size)
	{
		if (size == HullSize::Medium)
		{
			return 1;
		}

		if (size == HullSize::Large)
		{
			return 2;
		}

		return 0;
	}

	ShipClassDraft makeDraftFromChrome(const char* name, int hullSizeIndex, const ShipDesignPartCounts& parts)
	{
		return ShipClassDraft{
			.name = name,
			.hullSize = hullSizeFromIndex(hullSizeIndex),
			.parts = parts };
	}

	void partCountRow(const char* label, long long& count)
	{
		ImGui::PushID(label);
		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		if (ImGui::Button("-") && count > 0)
		{
			--count;
		}

		ImGui::SameLine();
		ImGui::Text("%lld", count);
		ImGui::SameLine();
		if (ImGui::Button("+"))
		{
			++count;
		}

		ImGui::PopID();
	}

}

	void DesignChrome::setOpen(bool open)
	{
		_open = open;
	}

	void DesignChrome::toggleOpen()
	{
		_open = !_open;
	}

	void DesignChrome::resetDraftToNewClass()
	{
		std::snprintf(_draftName, sizeof(_draftName), "New class");
		_hullSizeIndex = 0;
		_parts = ShipDesignPartCounts{ .reactorCount = 1, .engineCount = 1 };
	}

	void DesignChrome::draw(const WorldData& world, std::vector<GameCommand>& pendingCommands)
	{
		if (!_open)
		{
			return;
		}

		if (world.empires.empty())
		{
			return;
		}

		const long long empireId = world.empires.front().id;
		ImGui::SetNextWindowPos(ImVec2(24.0f, 56.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(720.0f, 420.0f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Ship design", &_open))
		{
			ImGui::End();
			return;
		}

		ImGui::BeginChild(
			"design_classes",
			ImVec2(240.0f, 0.0f),
			true);
		ImGui::TextUnformatted("Classes");
		if (ImGui::Button("New class"))
		{
			resetDraftToNewClass();
		}

		if (ImGui::BeginTable(
				"design_class_list",
				3,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("name");
			ImGui::TableSetupColumn("hull");
			ImGui::TableSetupColumn("mag");
			ImGui::TableHeadersRow();
			for (const auto& shipClass : world.shipClasses)
			{
				if (shipClass.empireId != empireId ||
					!shipClassHasRole(shipClass, HullRole::Warship))
				{
					continue;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				const std::string rowId = shipClass.name + "##" + std::to_string(shipClass.id);
				if (ImGui::Selectable(rowId.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
				{
					std::snprintf(_draftName, sizeof(_draftName), "%s", shipClass.name.c_str());
					_hullSizeIndex = hullSizeIndex(shipClass.hullSize);
					_parts.reactorCount = shipClass.reactorCount;
					_parts.engineCount = shipClass.engineCount;
					_parts.laserCount = shipClass.laserCount;
					_parts.missileTubeCount = shipClass.missileTubeCount;
					_parts.cargoCount = shipClass.cargoCount;
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted(hullSizeName(shipClass.hullSize));
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%lld", shipClass.magazineSize);
			}

			ImGui::EndTable();
		}

		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("design_draft", ImVec2(0.0f, 0.0f), true);
		ImGui::TextUnformatted("Draft");
		ImGui::InputText("Name", _draftName, sizeof(_draftName));
		if (ImGui::BeginCombo("Hull", hullSizeName(hullSizeFromIndex(_hullSizeIndex))))
		{
			if (ImGui::Selectable(hullSizeName(HullSize::Small), _hullSizeIndex == 0))
			{
				_hullSizeIndex = 0;
			}

			if (ImGui::Selectable(hullSizeName(HullSize::Medium), _hullSizeIndex == 1))
			{
				_hullSizeIndex = 1;
			}

			if (ImGui::Selectable(hullSizeName(HullSize::Large), _hullSizeIndex == 2))
			{
				_hullSizeIndex = 2;
			}

			ImGui::EndCombo();
		}

		partCountRow("Reactor", _parts.reactorCount);
		partCountRow("Engine", _parts.engineCount);
		partCountRow("Laser", _parts.laserCount);
		partCountRow("Missile tube", _parts.missileTubeCount);
		partCountRow("Cargo", _parts.cargoCount);
		const ShipClassDraft draft = makeDraftFromChrome(_draftName, _hullSizeIndex, _parts);
		const HullSizeSpec& hull = hullSizeSpec(draft.hullSize);
		ImGui::Text(
			"Slots %lld / %lld",
			shipDesignUsedSlots(draft.parts),
			hull.slots);
		ImGui::Text("Power %lld", shipDesignNetPower(draft.parts));
		ImGui::Text(
			"Magazine %lld",
			draft.parts.cargoCount * ShipDesignCargoMagazine);
		const ShipClassDraftFault fault = validateShipClassDraft(draft);
		if (fault != ShipClassDraftFault::None)
		{
			ImGui::TextUnformatted(shipClassDraftFaultName(fault));
		}

		const bool createDisabled = fault != ShipClassDraftFault::None;
		if (createDisabled)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Create"))
		{
			pendingCommands.push_back(GameCommand::createShipClass(
				empireId,
				draft,
				CommandSource::Player));
		}

		if (createDisabled)
		{
			ImGui::EndDisabled();
		}

		ImGui::EndChild();
		ImGui::End();
	}

}
