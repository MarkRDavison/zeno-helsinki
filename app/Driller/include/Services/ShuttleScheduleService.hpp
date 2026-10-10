#pragma once

#include <Core/Game.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Shuttle.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace drl
{

	class IShuttleScheduleService : public IGameTickService
	{
	public:
		~IShuttleScheduleService() override = 0;

		virtual bool moveShuttleTowardsLocation(
			float delta,
			ShuttleInstance& shuttle,
			const ShuttlePrototype& prototype,
			glm::vec2 target,
			ShuttleState nextState) = 0;
	};

	inline IShuttleScheduleService::~IShuttleScheduleService() = default;

	struct ShuttleCargoSale
	{
		std::vector<std::pair<std::string, long long>> sold;
		long long money{ 0 };
	};

	inline std::string resourceHudLabel(const IEconomyResourceService& economy, const std::string& name)
	{
		if (economy.hasHud(name))
		{
			return economy.getLabel(name);
		}

		constexpr std::string_view prefix = "Resource_";
		if (name.starts_with(prefix))
		{
			return name.substr(prefix.size());
		}

		return name;
	}

	inline std::string formatShuttleSaleMessage(
		const ShuttleCargoSale& sale,
		const IEconomyResourceService& economy)
	{
		std::string soldText;
		for (const auto& [name, amount] : sale.sold)
		{
			if (!soldText.empty())
			{
				soldText += ", ";
			}

			soldText += std::format("{} {}", amount, resourceHudLabel(economy, name));
		}

		return std::format(
			"Sold {} for {} {}",
			soldText,
			sale.money,
			resourceHudLabel(economy, ResourceMoney));
	}

	class ShuttleScheduleService : public IShuttleScheduleService
	{
	public:
		ShuttleScheduleService(
			ShuttleData& shuttleData,
			IWorkerRecruitmentService& recruitment,
			IWorkerCreationService& workerCreation,
			const IShuttlePrototypeService& shuttlePrototypes,
			IEconomyResourceService& economy);
		~ShuttleScheduleService() override = default;

		void update(float delta) override;
		void updateShuttle(float delta, ShuttleInstance& shuttle, const ShuttlePrototype& prototype);
		bool moveShuttleTowardsLocation(
			float delta,
			ShuttleInstance& shuttle,
			const ShuttlePrototype& prototype,
			glm::vec2 target,
			ShuttleState nextState) override;

		void updateShuttleOnArrivalAtSurface(ShuttleInstance& shuttle, const ShuttlePrototype& prototype);
		void updateShuttleOnArrivalAtDepartureDestination(ShuttleInstance& shuttle);
		bool consumeWorkerHousingShortage();
		std::optional<ShuttleCargoSale> consumeCargoSale();

	private:
		ShuttleData& _shuttleData;
		IWorkerRecruitmentService& _recruitment;
		IWorkerCreationService& _workerCreation;
		const IShuttlePrototypeService& _shuttlePrototypes;
		IEconomyResourceService& _economy;
		bool _workerHousingShortage{ false };
		std::optional<ShuttleCargoSale> _cargoSale;
	};

}
