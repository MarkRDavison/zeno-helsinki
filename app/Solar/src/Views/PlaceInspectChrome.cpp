#include <Views/PlaceInspectChrome.hpp>
#include <World/Goods.hpp>
#include <World/WorldData.hpp>
#include <imgui.h>
#include <algorithm>
#include <string>
#include <vector>

namespace slr
{
namespace
{

	void drawGoodQuantityTable(const char* tableId, const char* amountHeader, const std::vector<GoodQuantity>& rows)
	{
		if (rows.empty())
		{
			return;
		}

		if (!ImGui::BeginTable(tableId, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			return;
		}

		ImGui::TableSetupColumn("good");
		ImGui::TableSetupColumn(amountHeader);
		ImGui::TableHeadersRow();
		for (const auto& row : rows)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(goodName(row.goodId));
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(std::to_string(row.amount).c_str());
		}

		ImGui::EndTable();
	}

}

	void drawStarPlaceInspector(const SystemLayout& layout, const SystemPlace& place)
	{
		ImGui::TextUnformatted("Star");
		ImGui::TextUnformatted(("size " + std::to_string(layout.starSize)).c_str());
		ImGui::TextUnformatted(("out radius " + std::to_string(outRadiusForLayout(layout))).c_str());
		ImGui::TextUnformatted(
			("position " + std::to_string(place.position.x) + ", " + std::to_string(place.position.y)).c_str());
	}

	void drawJumpPlaceInspector(const WorldData& world, const SystemPlace& place)
	{
		ImGui::TextUnformatted("Jump");
		const StarSystem* neighbor = world.systemById(place.neighborSystemId);
		ImGui::TextUnformatted(neighbor != nullptr ? neighbor->name.c_str() : "unknown neighbour");
	}

	void drawMinePlaceInspector(const WorldData& world, long long systemId)
	{
		const StarSystem* system = world.systemById(systemId);
		int mineIndex = 0;
		for (const auto& mine : world.mines)
		{
			if (mine.systemId != systemId)
			{
				continue;
			}

			ImGui::TextUnformatted("Mine");
			const std::string mineTableId = "mine_" + std::to_string(mineIndex++);
			if (!mine.outputsPerTick.empty() &&
				ImGui::BeginTable(mineTableId.c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("good");
				ImGui::TableSetupColumn("stockpile");
				ImGui::TableSetupColumn("per tick");
				ImGui::TableSetupColumn("max");
				ImGui::TableHeadersRow();
				for (const auto& output : mine.outputsPerTick)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(goodName(output.goodId));
					ImGui::TableSetColumnIndex(1);
					const GoodAmount onHand =
						system != nullptr ? getStockpileAmount(*system, output.goodId) : 0.f;
					ImGui::TextUnformatted(std::to_string(onHand).c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::TextUnformatted(std::to_string(output.amountPerTick).c_str());
					ImGui::TableSetColumnIndex(3);
					ImGui::TextUnformatted(std::to_string(output.maxStockpile).c_str());
				}

				ImGui::EndTable();
			}
		}
	}

	void drawYardPlaceInspector(
		const WorldData& world,
		long long systemId,
		std::vector<GameCommand>& pendingCommands)
	{
		int yardIndex = 0;
		for (const auto& yard : world.yards)
		{
			if (yard.systemId != systemId)
			{
				continue;
			}

			ImGui::TextUnformatted("Yard");
			const std::string yardTableId = "yard_" + std::to_string(yardIndex++);
			drawGoodQuantityTable(yardTableId.c_str(), "desired", yard.desiredPiles);

			ImGui::TextUnformatted("Slips");
			const std::string slipsTableId = "yard_slips_" + yardTableId;
			if (ImGui::BeginTable(
					slipsTableId.c_str(),
					2,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("id");
				ImGui::TableSetupColumn("status");
				ImGui::TableHeadersRow();
				for (const auto& slip : yard.slips)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(std::to_string(slip.id).c_str());
					ImGui::TableSetColumnIndex(1);
					if (slip.remainingBuildTicks > 0)
					{
						ImGui::TextUnformatted(
							(std::to_string(slip.remainingBuildTicks) + " ticks left").c_str());
					}
					else if (slip.hullClassId != 0)
					{
						const ShipClass* hullClass = world.shipClassById(slip.hullClassId);
						const std::string hullName =
							hullClass != nullptr ? hullClass->name : ("class " + std::to_string(slip.hullClassId));
						if (slip.refitHullId != 0)
						{
							const ShipClass* fromClass = world.shipClassById(slip.refitFromClassId);
							const std::string fromName = fromClass != nullptr
								? fromClass->name
								: ("class " + std::to_string(slip.refitFromClassId));
							ImGui::TextUnformatted((fromName + " -> " + hullName + " " +
													std::to_string(slip.remainingHullBuildTicks) +
													" ticks left")
													   .c_str());
						}
						else
						{
							ImGui::TextUnformatted(
								(hullName + " " + std::to_string(slip.remainingHullBuildTicks) +
									" ticks left")
									.c_str());
						}
					}
					else
					{
						ImGui::TextUnformatted("idle");
					}
				}

				ImGui::EndTable();
			}

			ImGui::TextUnformatted("Build queue");
			const std::string queueTableId = "yard_queue_" + yardTableId;
			if (ImGui::BeginTable(
					queueTableId.c_str(),
					1,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("class");
				ImGui::TableHeadersRow();
				for (const long long classId : yard.hullBuildQueue)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					const ShipClass* queuedClass = world.shipClassById(classId);
					const std::string queuedName =
						queuedClass != nullptr ? queuedClass->name : ("class " + std::to_string(classId));
					ImGui::TextUnformatted(queuedName.c_str());
				}

				ImGui::EndTable();
			}

			ImGui::TextUnformatted("Refit queue");
			const std::string refitQueueTableId = "yard_refit_queue_" + yardTableId;
			if (ImGui::BeginTable(
					refitQueueTableId.c_str(),
					1,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("job");
				ImGui::TableHeadersRow();
				for (const auto& job : yard.hullRefitQueue)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					const ShipClass* fromClass = world.shipClassById(job.fromClassId);
					const ShipClass* toClass = world.shipClassById(job.toClassId);
					const std::string fromName =
						fromClass != nullptr ? fromClass->name : ("class " + std::to_string(job.fromClassId));
					const std::string toName =
						toClass != nullptr ? toClass->name : ("class " + std::to_string(job.toClassId));
					ImGui::TextUnformatted((fromName + " -> " + toName).c_str());
				}

				ImGui::EndTable();
			}

			ImGui::PushID(static_cast<int>(yard.systemId));
			const long long usableSteel = stockpileAvailableForYardConsume(world, systemId, SteelGoodId);
			const bool canStartYardSlip = usableSteel >= YardSlipSteelCost;
			if (!canStartYardSlip)
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Add slip"))
			{
				pendingCommands.push_back(GameCommand::startYardSlip(systemId, CommandSource::Player));
			}
			if (!canStartYardSlip)
			{
				ImGui::EndDisabled();
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(
				("usable steel " + std::to_string(usableSteel) + " / " + std::to_string(YardSlipSteelCost))
					.c_str());

			for (const auto& shipClass : world.shipClasses)
			{
				if (!shipClassIsTruckOfEmpire(shipClass, yard.empireId))
				{
					continue;
				}

				const std::string buttonId = "Queue " + shipClass.name + "##truck" + std::to_string(shipClass.id);
				if (ImGui::Button(buttonId.c_str()))
				{
					pendingCommands.push_back(GameCommand::startYardBuild(
						systemId,
						shipClass.id,
						CommandSource::Player));
				}
			}

			for (const auto& shipClass : world.shipClasses)
			{
				if (!shipClassIsWarshipOfEmpire(shipClass, yard.empireId))
				{
					continue;
				}

				const std::string buttonId = "Queue " + shipClass.name + "##warship" + std::to_string(shipClass.id);
				if (ImGui::Button(buttonId.c_str()))
				{
					pendingCommands.push_back(GameCommand::startYardBuild(
						systemId,
						shipClass.id,
						CommandSource::Player));
				}
			}

			std::vector<long long> parkedFromClassIds;
			for (const auto& fleet : world.fleets)
			{
				if (fleet.empireId != yard.empireId || fleet.systemId != systemId || !fleetIsParked(fleet))
				{
					continue;
				}

				for (const auto& hull : fleet.hulls)
				{
					const ShipClass* hullClass = world.shipClassById(hull.classId);
					if (hullClass == nullptr || !shipClassIsWarshipOfEmpire(*hullClass, yard.empireId))
					{
						continue;
					}

					if (std::find(
							parkedFromClassIds.begin(),
							parkedFromClassIds.end(),
							hull.classId) == parkedFromClassIds.end())
					{
						parkedFromClassIds.push_back(hull.classId);
					}
				}
			}

			std::sort(parkedFromClassIds.begin(), parkedFromClassIds.end());
			for (const long long fromClassId : parkedFromClassIds)
			{
				const ShipClass* fromClass = world.shipClassById(fromClassId);
				if (fromClass == nullptr)
				{
					continue;
				}

				for (const auto& toClass : world.shipClasses)
				{
					if (!shipClassIsWarshipOfEmpire(toClass, yard.empireId) ||
						toClass.id == fromClassId || toClass.hullSize != fromClass->hullSize)
					{
						continue;
					}

					const std::string buttonId = "Queue " + fromClass->name + " -> " + toClass.name +
						"##" + std::to_string(fromClassId) + "_" + std::to_string(toClass.id);
					if (ImGui::Button(buttonId.c_str()))
					{
						pendingCommands.push_back(GameCommand::startYardRefit(
							systemId,
							fromClassId,
							toClass.id,
							CommandSource::Player));
					}
				}
			}
			ImGui::PopID();

			if (yard.populationConsumePerTick.empty() && yard.populationReservedPiles.empty())
			{
				continue;
			}

			ImGui::TextUnformatted("Population");
			const std::string populationTableId = "yard_pop_" + yardTableId;
			if (ImGui::BeginTable(
					populationTableId.c_str(),
					3,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("good");
				ImGui::TableSetupColumn("consume/tick");
				ImGui::TableSetupColumn("reserved");
				ImGui::TableHeadersRow();

				std::vector<long long> populationGoodIds;
				auto addPopulationGoodId = [&populationGoodIds](long long goodId)
				{
					for (long long existing : populationGoodIds)
					{
						if (existing == goodId)
						{
							return;
						}
					}

					populationGoodIds.push_back(goodId);
				};

				for (const auto& row : yard.populationConsumePerTick)
				{
					addPopulationGoodId(row.goodId);
				}

				for (const auto& row : yard.populationReservedPiles)
				{
					addPopulationGoodId(row.goodId);
				}

				for (long long goodId : populationGoodIds)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(goodName(goodId));
					ImGui::TableSetColumnIndex(1);
					ImGui::TextUnformatted(
						std::to_string(quantityForGood(yard.populationConsumePerTick, goodId)).c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::TextUnformatted(
						std::to_string(quantityForGood(yard.populationReservedPiles, goodId)).c_str());
				}

				ImGui::EndTable();
			}
		}
	}

	void drawFactoryPlaceInspector(const WorldData& world, long long systemId)
	{
		const StarSystem* system = world.systemById(systemId);
		int factoryIndex = 0;
		for (const auto& factory : world.factories)
		{
			if (factory.systemId != systemId)
			{
				continue;
			}

			ImGui::TextUnformatted("Factory");
			const std::string factoryTableId = "factory_" + std::to_string(factoryIndex++);
			const ProductRampSpec* rampSpec = productRampSpecForGood(factory.outputGoodId);
			const int factoryColumnCount = rampSpec != nullptr ? 8 : 7;
			if (ImGui::BeginTable(
					factoryTableId.c_str(),
					factoryColumnCount,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("input");
				ImGui::TableSetupColumn("in stock");
				ImGui::TableSetupColumn("in/tick");
				ImGui::TableSetupColumn("output");
				ImGui::TableSetupColumn("out stock");
				ImGui::TableSetupColumn("out/tick");
				ImGui::TableSetupColumn("max");
				if (rampSpec != nullptr)
				{
					ImGui::TableSetupColumn("efficiency");
				}
				ImGui::TableHeadersRow();
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				{
					std::string inputNames;
					for (const auto& input : factory.inputsPerTick)
					{
						if (!inputNames.empty())
						{
							inputNames += "\n";
						}

						inputNames += goodName(input.goodId);
					}

					ImGui::TextUnformatted(inputNames.c_str());
				}
				ImGui::TableSetColumnIndex(1);
				{
					std::string inputStock;
					for (const auto& input : factory.inputsPerTick)
					{
						if (!inputStock.empty())
						{
							inputStock += "\n";
						}

						const GoodAmount onHand =
							system != nullptr ? getStockpileAmount(*system, input.goodId) : 0.f;
						inputStock += std::to_string(onHand);
					}

					ImGui::TextUnformatted(inputStock.c_str());
				}
				ImGui::TableSetColumnIndex(2);
				{
					std::string inputRates;
					for (const auto& input : factory.inputsPerTick)
					{
						if (!inputRates.empty())
						{
							inputRates += "\n";
						}

						inputRates += std::to_string(input.amount);
					}

					ImGui::TextUnformatted(inputRates.c_str());
				}
				ImGui::TableSetColumnIndex(3);
				ImGui::TextUnformatted(goodName(factory.outputGoodId));
				ImGui::TableSetColumnIndex(4);
				{
					const GoodAmount outputOnHand =
						system != nullptr ? getStockpileAmount(*system, factory.outputGoodId) : 0.f;
					ImGui::TextUnformatted(std::to_string(outputOnHand).c_str());
				}
				ImGui::TableSetColumnIndex(5);
				{
					std::string outRate = std::to_string(factory.outputAmountPerTick);
					if (factory.ticksPerOutput > 1)
					{
						outRate += " / " + std::to_string(factory.ticksPerOutput) + " ticks";
						if (factory.remainingOutputTicks > 0)
						{
							outRate += " (" + std::to_string(factory.remainingOutputTicks) + " left)";
						}
					}

					ImGui::TextUnformatted(outRate.c_str());
				}
				ImGui::TableSetColumnIndex(6);
				ImGui::TextUnformatted(std::to_string(factory.outputMaxStockpile).c_str());
				if (rampSpec != nullptr)
				{
					ImGui::TableSetColumnIndex(7);
					ImGui::TextUnformatted(
						(std::to_string(factory.productEfficiencyPerThousand / 10) + "%").c_str());
				}
				ImGui::EndTable();
			}
		}
	}

}
