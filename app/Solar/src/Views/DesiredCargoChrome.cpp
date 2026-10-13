#include <Views/DesiredCargoChrome.hpp>
#include <World/Goods.hpp>
#include <imgui.h>
#include <string>

namespace slr
{

	namespace
	{

		constexpr long long AddableCombatMissileGoodIds[] = { MissileGoodId, MissileLongRangeGoodId };

		bool draftContainsGood(const DesiredCargo& draft, long long goodId)
		{
			for (const auto& row : draft.rows)
			{
				if (row.goodId == goodId)
				{
					return true;
				}
			}

			return false;
		}

		bool draftHasFillRemaining(const DesiredCargo& draft)
		{
			for (const auto& row : draft.rows)
			{
				if (row.fill == DesiredCargoFill::FillRemaining)
				{
					return true;
				}
			}

			return false;
		}

	}

	bool DesiredCargoChrome::draw(DesiredCargo& draft, GoodAmount used, GoodAmount capacity)
	{
		bool changed = false;
		ImGui::TextUnformatted(
			("used " + std::to_string(static_cast<long long>(used)) + "/" +
				std::to_string(static_cast<long long>(capacity)))
				.c_str());

		int removeIndex = -1;
		for (int i = 0; i < static_cast<int>(draft.rows.size()); ++i)
		{
			DesiredCargoRow& row = draft.rows[static_cast<size_t>(i)];
			ImGui::PushID(i);
			ImGui::TextUnformatted(goodName(row.goodId));
			ImGui::SameLine();
			int fillIndex = row.fill == DesiredCargoFill::FillRemaining ? 1 : 0;
			const char* fillLabel = fillIndex == 1 ? "fill remaining" : "quota";
			if (ImGui::BeginCombo("##fill", fillLabel))
			{
				if (ImGui::Selectable("quota", fillIndex == 0) && fillIndex != 0)
				{
					row.fill = DesiredCargoFill::Quota;
					changed = true;
				}

				const bool canFillRemaining =
					fillIndex == 1 || !draftHasFillRemaining(draft);
				if (canFillRemaining &&
					ImGui::Selectable("fill remaining", fillIndex == 1) && fillIndex != 1)
				{
					row.fill = DesiredCargoFill::FillRemaining;
					changed = true;
				}

				ImGui::EndCombo();
			}

			if (row.fill == DesiredCargoFill::Quota)
			{
				int quota = static_cast<int>(row.quota);
				if (ImGui::InputInt("quota", &quota))
				{
					if (quota < 0)
					{
						quota = 0;
					}

					row.quota = static_cast<GoodAmount>(quota);
					changed = true;
				}
			}

			if (ImGui::Button("Remove"))
			{
				removeIndex = i;
			}

			ImGui::PopID();
		}

		if (removeIndex >= 0)
		{
			draft.rows.erase(draft.rows.begin() + removeIndex);
			changed = true;
		}

		if (ImGui::BeginCombo("Add good", "select"))
		{
			for (const long long goodId : AddableCombatMissileGoodIds)
			{
				if (draftContainsGood(draft, goodId))
				{
					continue;
				}

				if (ImGui::Selectable(goodName(goodId)))
				{
					const DesiredCargoFill fill = draftHasFillRemaining(draft)
						? DesiredCargoFill::Quota
						: DesiredCargoFill::FillRemaining;
					draft.rows.push_back(DesiredCargoRow{
						.goodId = goodId,
						.quota = 0.f,
						.fill = fill });
					changed = true;
				}
			}

			ImGui::EndCombo();
		}

		return changed;
	}

}
