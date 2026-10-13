#include <Views/ResearchChrome.hpp>
#include <World/Research.hpp>
#include <imgui.h>

namespace slr
{

	void ResearchChrome::setOpen(bool open)
	{
		_open = open;
	}

	void ResearchChrome::toggleOpen()
	{
		_open = !_open;
	}

	void ResearchChrome::draw(const WorldData& world, std::vector<GameCommand>& pendingCommands)
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
		const long long activeCount = countActiveResearchProjects(world, empireId);
		const ResearchAmount share = researchPointsAddedPerActiveProject(activeCount);

		ImGui::SetNextWindowPos(ImVec2(24.0f, 56.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(520.0f, 360.0f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Research", &_open))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginTabBar("research_lists"))
		{
			if (ImGui::BeginTabItem("Available"))
			{
				if (ImGui::BeginTable(
						"research_available",
						3,
						ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
				{
					ImGui::TableSetupColumn("name");
					ImGui::TableSetupColumn("progress");
					ImGui::TableSetupColumn("action");
					ImGui::TableHeadersRow();
					for (const auto& spec : researchCatalogue())
					{
						const ResearchProgress* row =
							world.researchProgressByEmpireAndProject(empireId, spec.id);
						if (row != nullptr && (row->active || row->completed))
						{
							continue;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(spec.name);
						ImGui::TableSetColumnIndex(1);
						const ResearchAmount progress = row != nullptr ? row->progress : 0.f;
						ImGui::Text(
							"%lld / %lld",
							researchDisplayedAmount(progress),
							researchDisplayedAmount(spec.cost));
						ImGui::TableSetColumnIndex(2);
						ImGui::PushID(static_cast<int>(spec.id));
						if (ImGui::Button("Start"))
						{
							pendingCommands.push_back(GameCommand::startResearchProject(
								empireId,
								spec.id,
								CommandSource::Player));
						}

						ImGui::PopID();
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Active"))
			{
				ImGui::Text(
					"Share this tick %lld",
					researchDisplayedAmount(share));
				if (ImGui::BeginTable(
						"research_active",
						4,
						ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
				{
					ImGui::TableSetupColumn("name");
					ImGui::TableSetupColumn("progress");
					ImGui::TableSetupColumn("share");
					ImGui::TableSetupColumn("action");
					ImGui::TableHeadersRow();
					for (const auto& spec : researchCatalogue())
					{
						const ResearchProgress* row =
							world.researchProgressByEmpireAndProject(empireId, spec.id);
						if (row == nullptr || !row->active || row->completed)
						{
							continue;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(spec.name);
						ImGui::TableSetColumnIndex(1);
						ImGui::Text(
							"%lld / %lld",
							researchDisplayedAmount(row->progress),
							researchDisplayedAmount(spec.cost));
						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%lld", researchDisplayedAmount(share));
						ImGui::TableSetColumnIndex(3);
						ImGui::PushID(static_cast<int>(spec.id));
						if (ImGui::Button("Stop"))
						{
							pendingCommands.push_back(GameCommand::stopResearchProject(
								empireId,
								spec.id,
								CommandSource::Player));
						}

						ImGui::PopID();
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Completed"))
			{
				if (ImGui::BeginTable(
						"research_completed",
						2,
						ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
				{
					ImGui::TableSetupColumn("name");
					ImGui::TableSetupColumn("progress");
					ImGui::TableHeadersRow();
					for (const auto& spec : researchCatalogue())
					{
						const ResearchProgress* row =
							world.researchProgressByEmpireAndProject(empireId, spec.id);
						if (row == nullptr || !row->completed)
						{
							continue;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(spec.name);
						ImGui::TableSetColumnIndex(1);
						ImGui::Text(
							"%lld / %lld",
							researchDisplayedAmount(row->progress),
							researchDisplayedAmount(spec.cost));
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

}
