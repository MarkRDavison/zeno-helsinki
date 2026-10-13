#include <Views/EngagementPolicyChrome.hpp>
#include <imgui.h>
#include <cstdio>
#include <string>

namespace slr
{
namespace
{

	int ruleOfEngagementComboIndex(RuleOfEngagement rule)
	{
		return rule == RuleOfEngagement::FireIfEnemySeen ? 1 : 0;
	}

	RuleOfEngagement ruleOfEngagementFromComboIndex(int index)
	{
		return index == 1 ? RuleOfEngagement::FireIfEnemySeen
						  : RuleOfEngagement::DoNotFireUnlessFiredUpon;
	}

	int fleetApproachComboIndex(FleetApproach approach)
	{
		return approach == FleetApproach::HoldMissileRange ? 1 : 0;
	}

	FleetApproach fleetApproachFromComboIndex(int index)
	{
		return index == 1 ? FleetApproach::HoldMissileRange : FleetApproach::CloseToLaserRange;
	}

}

	void EngagementPolicyChrome::setOpen(bool open)
	{
		_open = open;
	}

	void EngagementPolicyChrome::draw(const WorldData& world, std::vector<GameCommand>& pendingCommands)
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
		ImGui::SetNextWindowSize(ImVec2(480.0f, 400.0f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Engagement policies", &_open))
		{
			ImGui::End();
			return;
		}

		ImGui::TextUnformatted("Policies");
		if (ImGui::BeginTable(
				"engagement_policies",
				3,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("name");
			ImGui::TableSetupColumn("ROE");
			ImGui::TableSetupColumn("approach");
			ImGui::TableHeadersRow();
			for (const auto& policy : world.engagementPolicies)
			{
				if (policy.empireId != empireId)
				{
					continue;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				const bool rowSelected = _draftId == policy.id;
				const std::string rowId = policy.name + "##" + std::to_string(policy.id);
				if (ImGui::Selectable(
						rowId.c_str(),
						rowSelected,
						ImGuiSelectableFlags_SpanAllColumns))
				{
					_draftId = policy.id;
					std::snprintf(_draftName, sizeof(_draftName), "%s", policy.name.c_str());
					_draftRuleIndex = ruleOfEngagementComboIndex(policy.ruleOfEngagement);
					_draftApproachIndex = fleetApproachComboIndex(policy.approach);
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted(ruleOfEngagementName(policy.ruleOfEngagement));
				ImGui::TableSetColumnIndex(2);
				ImGui::TextUnformatted(fleetApproachName(policy.approach));
			}

			ImGui::EndTable();
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Civilians");
		for (const auto& empire : world.empires)
		{
			if (empire.id == empireId)
			{
				continue;
			}

			bool forbidden = civilianHaulIsForbidden(world, empireId, empire.id);
			const std::string label = "Forbid civilians on " + empire.name + " claims";
			if (ImGui::Checkbox(label.c_str(), &forbidden))
			{
				pendingCommands.push_back(GameCommand::setCivilianHaulForbidden(
					empireId,
					empire.id,
					forbidden,
					CommandSource::Player));
			}
		}

		if (ImGui::Button("New policy"))
		{
				pendingCommands.push_back(GameCommand::upsertEngagementPolicy(
				0,
				empireId,
				"New policy",
				RuleOfEngagement::DoNotFireUnlessFiredUpon,
				CommandSource::Player,
				FleetApproach::CloseToLaserRange));
		}

		if (_draftId != 0)
		{
			ImGui::InputText("Name", _draftName, sizeof(_draftName));
			if (ImGui::BeginCombo(
					"Rule",
					ruleOfEngagementName(ruleOfEngagementFromComboIndex(_draftRuleIndex))))
			{
				if (ImGui::Selectable(
						ruleOfEngagementName(RuleOfEngagement::DoNotFireUnlessFiredUpon),
						_draftRuleIndex == 0))
				{
					_draftRuleIndex = 0;
				}

				if (ImGui::Selectable(
						ruleOfEngagementName(RuleOfEngagement::FireIfEnemySeen),
						_draftRuleIndex == 1))
				{
					_draftRuleIndex = 1;
				}

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Approach", fleetApproachName(fleetApproachFromComboIndex(_draftApproachIndex))))
			{
				if (ImGui::Selectable(
						fleetApproachName(FleetApproach::CloseToLaserRange),
						_draftApproachIndex == 0))
				{
					_draftApproachIndex = 0;
				}

				if (ImGui::Selectable(
						fleetApproachName(FleetApproach::HoldMissileRange),
						_draftApproachIndex == 1))
				{
					_draftApproachIndex = 1;
				}

				ImGui::EndCombo();
			}

			if (ImGui::Button("Save"))
			{
				pendingCommands.push_back(GameCommand::upsertEngagementPolicy(
					_draftId,
					empireId,
					_draftName,
					ruleOfEngagementFromComboIndex(_draftRuleIndex),
					CommandSource::Player,
					fleetApproachFromComboIndex(_draftApproachIndex)));
			}

			ImGui::SameLine();
			if (ImGui::Button("Set as default"))
			{
				pendingCommands.push_back(GameCommand::setEmpireDefaultPolicy(
					empireId,
					_draftId,
					CommandSource::Player));
			}
		}

		ImGui::End();
	}

}
