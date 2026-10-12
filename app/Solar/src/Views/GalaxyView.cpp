#include <Views/GalaxyView.hpp>
#include <GalaxyPushConstantObject.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/PickFleet.hpp>
#include <World/PickTruck.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/glm.hpp>
#include <imgui.h>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace slr
{
namespace
{
	const glm::vec4 NodeColour{ 0.75f, 0.85f, 1.0f, 1.0f };
	const glm::vec4 SelectedNodeColour{ 1.0f, 0.95f, 0.75f, 1.0f };
	const glm::vec4 LaneColour{ 0.28f, 0.36f, 0.48f, 1.0f };
	const glm::vec4 TruckColour{ 0.95f, 0.65f, 0.22f, 1.0f };
	const glm::vec4 SelectedTruckColour{ 1.0f, 0.85f, 0.45f, 1.0f };
	const glm::vec4 FleetColour{ 0.45f, 0.82f, 0.62f, 1.0f };
	const glm::vec4 SelectedFleetColour{ 0.7f, 0.95f, 0.8f, 1.0f };
	const glm::vec4 RivalFleetColour{ 0.85f, 0.38f, 0.4f, 1.0f };
	const glm::vec4 SelectedRivalFleetColour{ 1.0f, 0.55f, 0.55f, 1.0f };

	bool isPlayerFleet(const WorldData& world, const Fleet& fleet)
	{
		return world.empires.empty() || fleet.empireId == world.empires.front().id;
	}

	glm::vec4 fleetDrawColour(const WorldData& world, const Fleet& fleet, bool selected)
	{
		if (isPlayerFleet(world, fleet))
		{
			return selected ? SelectedFleetColour : FleetColour;
		}

		return selected ? SelectedRivalFleetColour : RivalFleetColour;
	}

	std::string convoyTrucksPresentOfTotal(const WorldData& world, const Convoy& convoy)
	{
		return std::to_string(countTrucksInConvoyPresent(world, convoy)) + "/" +
			std::to_string(countTrucksInConvoy(world, convoy.id));
	}

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

	void drawUnitQuad(
		hl::PipelineDrawData& pdd,
		const hl::VertexArrayResource& quad,
		const glm::mat4& model,
		const glm::vec4& color,
		int cameraIndex)
	{
		GalaxyPushConstantObject pc{
			.model = model,
			.color = color,
			.cameraIndex = cameraIndex
		};

		vkCmdPushConstants(
			pdd.commandBuffer,
			pdd.pipeline->getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(GalaxyPushConstantObject),
			&pc);

		VkBuffer vertexBuffers[] = { quad._vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);

		auto descriptorSet = pdd.pipeline->getDescriptorSet(pdd.currentFrame);
		vkCmdBindDescriptorSets(
			pdd.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pdd.pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDraw(pdd.commandBuffer, 6, 1, 0, 0);
	}

	void drawSdfText(
		hl::PipelineDrawData& pdd,
		hl::TextSystem& textSystem,
		int textId,
		glm::vec2 pos,
		int cameraIndex,
		const glm::vec4& colour)
	{
		const auto& t = textSystem.getText(textId);
		if (t._fontType != hl::FontType::SignedDistanceField)
		{
			return;
		}

		auto pc = hl::TextPushConstantObject{
			.model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)),
			.colour = colour,
			.fontAtlasIndex = textSystem.getFontAtlasIndex(hl::FontType::SignedDistanceField, "roboto"),
			.cameraIndex = cameraIndex
		};

		vkCmdPushConstants(
			pdd.commandBuffer,
			pdd.pipeline->getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(hl::TextPushConstantObject),
			&pc);

		VkBuffer vertexBuffers[] = { t._vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);

		auto descriptorSet = pdd.pipeline->getDescriptorSet(pdd.currentFrame);
		vkCmdBindDescriptorSets(
			pdd.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pdd.pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDraw(pdd.commandBuffer, t._vertexCount, 1, 0, 0);
	}
}

	void GalaxyView::initialise(hl::TextSystem& textSystem, const WorldData& world, int galaxyCameraIndex)
	{
		_world = &world;
		_galaxyCameraIndex = galaxyCameraIndex;
		_labels.clear();
		_labels.reserve(world.systems.size());
		_selectedSystemId.reset();
		_selectedFleetId.reset();
		_selectedTruckId.reset();

		for (const auto& system : world.systems)
		{
			Label label;
			label.textId = textSystem.registerText(system.name, "roboto", 24);
			label.worldCenter = system.position + glm::vec2(0.0f, GalaxyLabelOffsetY);
			_labels.push_back(label);
		}
	}

	void GalaxyView::setSelectedSystemId(std::optional<long long> id)
	{
		_selectedFleetId.reset();
		_selectedTruckId.reset();
		_selectedSystemId = id;
		if (_selectedSystemId.has_value() && _world != nullptr && _world->systemById(*_selectedSystemId) == nullptr)
		{
			_selectedSystemId.reset();
		}
	}

	void GalaxyView::setSelectedFleetId(std::optional<long long> id)
	{
		_selectedSystemId.reset();
		_selectedTruckId.reset();
		_selectedFleetId = id;
		if (_selectedFleetId.has_value() && _world != nullptr && _world->fleetById(*_selectedFleetId) == nullptr)
		{
			_selectedFleetId.reset();
		}
	}

	void GalaxyView::setSelectedTruckId(std::optional<long long> id)
	{
		_selectedSystemId.reset();
		_selectedFleetId.reset();
		_selectedTruckId = id;
		if (_selectedTruckId.has_value() && _world != nullptr && _world->truckById(*_selectedTruckId) == nullptr)
		{
			_selectedTruckId.reset();
		}
	}

	void GalaxyView::drawGraph(hl::PipelineDrawData& pdd, hl::ResourceManager& resourceManager) const
	{
		if (_world == nullptr)
		{
			return;
		}

		const auto& quad = *resourceManager.GetResource<hl::VertexArrayResource>("unit_quad");

		for (const auto& lane : _world->hyperlanes)
		{
			const StarSystem* from = _world->systemById(lane.a);
			const StarSystem* to = _world->systemById(lane.b);
			if (from == nullptr || to == nullptr)
			{
				continue;
			}

			const glm::vec2 delta = to->position - from->position;
			const float length = glm::length(delta);
			if (length <= 0.0f)
			{
				continue;
			}

			const float angle = std::atan2(delta.y, delta.x);
			const glm::vec2 mid = (from->position + to->position) * 0.5f;
			const glm::mat4 model =
				glm::translate(glm::mat4(1.0f), glm::vec3(mid, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(length, GalaxyLaneThickness, 1.0f));
			drawUnitQuad(pdd, quad, model, LaneColour, _galaxyCameraIndex);
		}

		for (const auto& fleet : _world->fleets)
		{
			if (fleetShipCount(fleet) == 0)
			{
				continue;
			}

			const auto marker = fleetMarkerPosition(*_world, fleet);
			if (!marker.has_value())
			{
				continue;
			}

			const bool selected = _selectedFleetId.has_value() && fleet.id == *_selectedFleetId;
			const glm::mat4 model =
				glm::translate(glm::mat4(1.0f), glm::vec3(*marker, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(GalaxyFleetSize, GalaxyFleetSize, 1.0f));
			drawUnitQuad(
				pdd,
				quad,
				model,
				fleetDrawColour(*_world, fleet, selected),
				_galaxyCameraIndex);
		}

		for (const auto& system : _world->systems)
		{
			const bool selected = _selectedSystemId.has_value() && system.id == *_selectedSystemId;
			const glm::mat4 model =
				glm::translate(glm::mat4(1.0f), glm::vec3(system.position, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(GalaxyNodeSize, GalaxyNodeSize, 1.0f));
			drawUnitQuad(
				pdd,
				quad,
				model,
				selected ? SelectedNodeColour : NodeColour,
				_galaxyCameraIndex);
		}

		for (const auto& truck : _world->trucks)
		{
			const auto pos = truckMarkerPosition(*_world, truck);
			if (!pos.has_value())
			{
				continue;
			}

			const bool selected = _selectedTruckId.has_value() && truck.id == *_selectedTruckId;
			const glm::mat4 model =
				glm::translate(glm::mat4(1.0f), glm::vec3(*pos, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(GalaxyTruckSize, GalaxyTruckSize, 1.0f));
			drawUnitQuad(
				pdd,
				quad,
				model,
				selected ? SelectedTruckColour : TruckColour,
				_galaxyCameraIndex);
		}
	}

	void GalaxyView::drawLabels(hl::PipelineDrawData& pdd, hl::TextSystem& textSystem) const
	{
		for (const auto& label : _labels)
		{
			const auto& size = textSystem.getTextSize(label.textId);
			glm::vec2 pos = label.worldCenter;
			pos.x += size.x - size.z / 2.0f;
			pos.y += size.y - size.w / 2.0f;
			drawSdfText(
				pdd,
				textSystem,
				label.textId,
				pos,
				_galaxyCameraIndex,
				glm::vec4(0.9f, 0.92f, 0.95f, 1.0f));
		}
	}

	bool GalaxyView::drawInspector(std::vector<GameCommand>& pendingCommands)
	{
		if (_world == nullptr)
		{
			return true;
		}

		if (_selectedTruckId.has_value())
		{
			const Truck* truck = _world->truckById(*_selectedTruckId);
			if (truck == nullptr)
			{
				return false;
			}

			bool open = true;
			ImGui::SetNextWindowPos(ImVec2(24.0f, 56.0f), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Truck", &open))
			{
				ImGui::TextUnformatted(("id " + std::to_string(truck->id)).c_str());
				const Empire* empire = _world->empireById(truck->empireId);
				if (empire != nullptr)
				{
					ImGui::TextUnformatted(empire->name.c_str());
				}

				const HaulJob* job = _world->haulJobById(truck->haulJobId);
				if (job != nullptr)
				{
					const StarSystem* dest = _world->systemById(job->destinationSystemId);
					if (dest != nullptr)
					{
						ImGui::TextUnformatted(("job dest " + dest->name).c_str());
					}
				}

				if (truck->cargoAmount > 0)
				{
					ImGui::TextUnformatted(
						(std::string(goodName(truck->cargoGoodId)) + " " +
							std::to_string(truck->cargoAmount))
							.c_str());
				}
				else
				{
					ImGui::TextUnformatted("empty");
				}

				if (truck->convoyId == 0)
				{
					ImGui::TextUnformatted("Convoy");
					if (ImGui::BeginTable(
							"truck_convoy",
							2,
							ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						ImGui::TableSetupColumn("trucks");
						ImGui::TableSetupColumn("status");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted("0");
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted("not in a convoy");
						ImGui::EndTable();
					}
				}
				else
				{
					const Convoy* convoy = _world->convoyById(truck->convoyId);
					ImGui::TextUnformatted("Convoy");
					if (ImGui::BeginTable(
							"truck_convoy",
							3,
							ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						ImGui::TableSetupColumn("trucks");
						ImGui::TableSetupColumn("amount");
						ImGui::TableSetupColumn("status");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						if (convoy != nullptr)
						{
							const std::string trucks = convoyTrucksPresentOfTotal(*_world, *convoy);
							ImGui::TextUnformatted(trucks.c_str());
						}
						else
						{
							ImGui::TextUnformatted("0/0");
						}
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(
							convoy != nullptr
								? std::to_string(convoyCargoOrReservedAmount(*_world, *convoy)).c_str()
								: "0");
						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(
							convoy != nullptr && convoyIsUnderway(*convoy) ? "underway" : "assembling");
						ImGui::EndTable();
					}

					if (convoy != nullptr)
					{
						ImGui::TextUnformatted(
							convoyIsStanding(*convoy) ? "standing" : "job group");
						if (convoyIsStanding(*convoy))
						{
							const Fleet* escort = _world->fleetById(convoy->escortFleetId);
							if (escort != nullptr)
							{
								ImGui::TextUnformatted(("escort " + escort->name).c_str());
							}
						}
					}
				}
			}
			ImGui::End();
			return open;
		}

		if (_selectedFleetId.has_value())
		{
			const Fleet* fleet = _world->fleetById(*_selectedFleetId);
			if (fleet == nullptr)
			{
				return false;
			}

			const StarSystem* at = _world->systemById(fleet->systemId);
			bool open = true;
			ImGui::SetNextWindowPos(ImVec2(24.0f, 56.0f), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Fleet", &open))
			{
			ImGui::TextUnformatted(fleet->name.c_str());
			ImGui::TextUnformatted(("id " + std::to_string(fleet->id)).c_str());
			const Empire* empire = _world->empireById(fleet->empireId);
			if (empire != nullptr)
			{
				ImGui::TextUnformatted(empire->name.c_str());
			}
				if (fleet->motion.route.size() >= 2)
				{
					const StarSystem* dest = _world->systemById(fleet->motion.route.back());
					if (dest != nullptr)
					{
						ImGui::TextUnformatted(("en route to " + dest->name).c_str());
					}
				}
				else if (at != nullptr)
				{
					ImGui::TextUnformatted(("at " + at->name).c_str());
				}
				for (const auto& row : fleet->composition)
				{
					const ShipClass* shipClass = _world->shipClassById(row.classId);
					if (shipClass == nullptr)
					{
						continue;
					}

					ImGui::TextUnformatted(
						(shipClass->name + " " + std::to_string(row.count)).c_str());
				}

				const EngagementPolicy* active = activeEngagementPolicy(*_world, *fleet);
				const char* activeRule = active != nullptr
					? ruleOfEngagementName(active->ruleOfEngagement)
					: "none";
				const std::string activeLabel = fleet->policyId == 0
					? std::string("default: ") + (active != nullptr ? active->name : "none")
					: (active != nullptr ? active->name : "none");
				ImGui::TextUnformatted(("ROE " + activeLabel).c_str());
				ImGui::TextUnformatted(activeRule);

				if (isPlayerFleet(*_world, *fleet))
				{
					const Empire* playerEmpire = _world->empireById(fleet->empireId);
					std::string defaultCombo = "Empire default";
					if (playerEmpire != nullptr)
					{
						const EngagementPolicy* defaultPolicy =
							_world->engagementPolicyById(playerEmpire->defaultPolicyId);
						if (defaultPolicy != nullptr)
						{
							defaultCombo += " (" + defaultPolicy->name + ")";
						}
					}

					if (ImGui::BeginCombo("Policy", activeLabel.c_str()))
					{
						if (ImGui::Selectable(defaultCombo.c_str(), fleet->policyId == 0))
						{
							pendingCommands.push_back(GameCommand::setFleetPolicyOverride(
								fleet->id,
								0,
								CommandSource::Player));
						}

						for (const auto& policy : _world->engagementPolicies)
						{
							if (policy.empireId != fleet->empireId)
							{
								continue;
							}

							const bool selected = fleet->policyId == policy.id;
							const std::string itemId = policy.name + "##p" + std::to_string(policy.id);
							if (ImGui::Selectable(itemId.c_str(), selected))
							{
								pendingCommands.push_back(GameCommand::setFleetPolicyOverride(
									fleet->id,
									policy.id,
									CommandSource::Player));
							}
						}

						ImGui::EndCombo();
					}

					const Convoy* escortConvoy = _world->convoyById(fleet->escortConvoyId);
					const bool escorting = escortConvoy != nullptr && convoyIsStanding(*escortConvoy);
					const char* stanceLabel = escorting ? "Escort" : "Front";
					if (ImGui::BeginCombo("Stance", stanceLabel))
					{
						if (ImGui::Selectable("Front", !escorting) && escorting)
						{
							pendingCommands.push_back(GameCommand::setFleetEscort(
								fleet->id,
								0,
								CommandSource::Player));
						}

						if (ImGui::Selectable("Escort", escorting) && !escorting)
						{
							pendingCommands.push_back(GameCommand::setFleetEscort(
								fleet->id,
								1,
								CommandSource::Player));
						}

						ImGui::EndCombo();
					}

					if (escorting)
					{
						int trucksWanted = static_cast<int>(escortConvoy->desiredTruckCount);
						if (ImGui::InputInt("Trucks", &trucksWanted))
						{
							if (trucksWanted < 0)
							{
								trucksWanted = 0;
							}

							pendingCommands.push_back(GameCommand::setFleetEscort(
								fleet->id,
								trucksWanted,
								CommandSource::Player));
						}

						ImGui::TextUnformatted(
							("joined " +
								std::to_string(countTrucksInConvoy(*_world, escortConvoy->id)) +
								"/" +
								std::to_string(escortConvoy->desiredTruckCount))
								.c_str());
					}
				}
			}
			ImGui::End();
			return open;
		}

		if (!_selectedSystemId.has_value())
		{
			return true;
		}

		const StarSystem* system = _world->systemById(*_selectedSystemId);
		if (system == nullptr)
		{
			return false;
		}

		bool open = true;
		ImGui::SetNextWindowPos(ImVec2(24.0f, 56.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("System", &open))
		{
			ImGui::TextUnformatted(system->name.c_str());
			ImGui::TextUnformatted(("id " + std::to_string(system->id)).c_str());
			ImGui::TextUnformatted(
				("trucks " + std::to_string(countTrucksAtSystem(*_world, system->id))).c_str());
			std::vector<GoodQuantity> haulJobRows;
			for (const auto& job : _world->haulJobs)
			{
				if (job.destinationSystemId == system->id)
				{
					haulJobRows.push_back(GoodQuantity{ .goodId = job.goodId, .amount = job.amount });
				}
			}
			if (!haulJobRows.empty())
			{
				ImGui::TextUnformatted("Haul jobs");
				drawGoodQuantityTable("haul_jobs", "amount", haulJobRows);
			}
			bool anyConvoy = false;
			for (const auto& convoy : _world->convoys)
			{
				const HaulJob* job = _world->haulJobById(convoy.haulJobId);
				if (convoy.loadSystemId == system->id ||
					(job != nullptr && job->destinationSystemId == system->id))
				{
					anyConvoy = true;
					break;
				}
			}
			if (anyConvoy)
			{
				ImGui::TextUnformatted("Convoys");
				if (ImGui::BeginTable("convoys", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("trucks");
					ImGui::TableSetupColumn("good");
					ImGui::TableSetupColumn("amount");
					ImGui::TableSetupColumn("status");
					ImGui::TableHeadersRow();
					for (const auto& convoy : _world->convoys)
					{
						const HaulJob* job = _world->haulJobById(convoy.haulJobId);
						if (convoy.loadSystemId != system->id &&
							(job == nullptr || job->destinationSystemId != system->id))
						{
							continue;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						const std::string trucks = convoyTrucksPresentOfTotal(*_world, convoy);
						ImGui::TextUnformatted(trucks.c_str());
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(job != nullptr ? goodName(job->goodId) : "");
						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(
							std::to_string(convoyCargoOrReservedAmount(*_world, convoy)).c_str());
						ImGui::TableSetColumnIndex(3);
						ImGui::TextUnformatted(convoyIsUnderway(convoy) ? "underway" : "assembling");
					}
					ImGui::EndTable();
				}
			}
			if (!system->stocks.empty())
			{
				ImGui::TextUnformatted("Stockpile");
				if (ImGui::BeginTable("system_goods", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("good");
					ImGui::TableSetupColumn("stockpile");
					ImGui::TableSetupColumn("inbound");
					ImGui::TableSetupColumn("need");
					ImGui::TableSetupColumn("desired");
					ImGui::TableHeadersRow();
					for (const auto& stock : system->stocks)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(goodName(stock.goodId));
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(std::to_string(stock.goodAmount).c_str());
						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(std::to_string(stock.inbound).c_str());
						ImGui::TableSetColumnIndex(3);
						ImGui::TextUnformatted(std::to_string(stock.need).c_str());
						ImGui::TableSetColumnIndex(4);
						ImGui::TextUnformatted(
							std::to_string(consumerDesiredPile(*_world, system->id, stock.goodId)).c_str());
					}

					ImGui::EndTable();
				}
			}

			int mineIndex = 0;
			for (const auto& mine : _world->mines)
			{
				if (mine.systemId != system->id)
				{
					continue;
				}

				ImGui::TextUnformatted("Mine");
				const std::string mineTableId = "mine_" + std::to_string(mineIndex++);
				if (!mine.outputsPerTick.empty() &&
					ImGui::BeginTable(mineTableId.c_str(), 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("good");
					ImGui::TableSetupColumn("per tick");
					ImGui::TableSetupColumn("max");
					ImGui::TableHeadersRow();
					for (const auto& output : mine.outputsPerTick)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(goodName(output.goodId));
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(std::to_string(output.amountPerTick).c_str());
						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(std::to_string(output.maxStockpile).c_str());
					}

					ImGui::EndTable();
				}
			}

			int yardIndex = 0;
			for (const auto& yard : _world->yards)
			{
				if (yard.systemId != system->id)
				{
					continue;
				}

				ImGui::TextUnformatted("Yard");
				const std::string yardTableId = "yard_" + std::to_string(yardIndex++);
				drawGoodQuantityTable(yardTableId.c_str(), "desired", yard.desiredPiles);

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

			int factoryIndex = 0;
			for (const auto& factory : _world->factories)
			{
				if (factory.systemId != system->id)
				{
					continue;
				}

				ImGui::TextUnformatted("Factory");
				const std::string factoryTableId = "factory_" + std::to_string(factoryIndex++);
				if (ImGui::BeginTable(
						factoryTableId.c_str(),
						5,
						ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("input");
					ImGui::TableSetupColumn("in/tick");
					ImGui::TableSetupColumn("output");
					ImGui::TableSetupColumn("out/tick");
					ImGui::TableSetupColumn("max");
					ImGui::TableHeadersRow();
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(goodName(factory.inputGoodId));
					ImGui::TableSetColumnIndex(1);
					ImGui::TextUnformatted(std::to_string(factory.inputAmountPerTick).c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::TextUnformatted(goodName(factory.outputGoodId));
					ImGui::TableSetColumnIndex(3);
					ImGui::TextUnformatted(std::to_string(factory.outputAmountPerTick).c_str());
					ImGui::TableSetColumnIndex(4);
					ImGui::TextUnformatted(std::to_string(factory.outputMaxStockpile).c_str());
					ImGui::EndTable();
				}
			}
		}
		ImGui::End();
		return open;
	}

	void GalaxyView::drawBattleReport()
	{
		if (_world == nullptr || _world->battleReports.empty())
		{
			return;
		}

		const BattleReport& report = _world->battleReports.back();
		if (_dismissedReportId.has_value() && *_dismissedReportId == report.id)
		{
			return;
		}

		bool open = true;
		ImGui::SetNextWindowPos(ImVec2(24.0f, 220.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Battle report", &open))
		{
			const StarSystem* system = _world->systemById(report.systemId);
			if (system != nullptr)
			{
				ImGui::TextUnformatted(("at " + system->name).c_str());
			}

			for (const auto& side : report.sides)
			{
				const Fleet* fleet = _world->fleetById(side.fleetId);
				if (fleet != nullptr)
				{
					ImGui::TextUnformatted(fleet->name.c_str());
				}
				else
				{
					ImGui::TextUnformatted(("fleet " + std::to_string(side.fleetId)).c_str());
				}

				std::vector<long long> classIds;
				for (const auto& row : side.before)
				{
					classIds.push_back(row.classId);
				}
				for (const auto& row : side.after)
				{
					if (classCountFor(side.before, row.classId) == 0)
					{
						classIds.push_back(row.classId);
					}
				}

				for (const long long classId : classIds)
				{
					const ShipClass* shipClass = _world->shipClassById(classId);
					const std::string name = shipClass != nullptr ? shipClass->name : ("class " + std::to_string(classId));
					const long long before = classCountFor(side.before, classId);
					const long long after = classCountFor(side.after, classId);
					ImGui::TextUnformatted(
						(name + " " + std::to_string(before) + " -> " + std::to_string(after)).c_str());
				}
			}
		}

		ImGui::End();
		if (!open)
		{
			_dismissedReportId = report.id;
		}
	}

}
