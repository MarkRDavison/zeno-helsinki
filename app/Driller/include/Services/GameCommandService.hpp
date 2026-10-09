#pragma once

#include <Core/GameCommand.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/TerrainAlterationService.hpp>

namespace drl
{

	inline constexpr long long kPlayerShaftDigCostPerLevel = 100;

	class IGameCommandService
	{
	public:
		virtual ~IGameCommandService() = 0;

		virtual bool execute(const GameCommand& command) = 0;
		virtual void tick() = 0;
		virtual long long currentTick() const = 0;
	};

	inline IGameCommandService::~IGameCommandService() = default;

	class GameCommandService : public IGameCommandService
	{
	public:
		GameCommandService(
			ITerrainAlterationService& terrain,
			IEconomyResourceService& economy);
		~GameCommandService() override = default;

		bool execute(const GameCommand& command) override;
		void tick() override;
		long long currentTick() const override;

	private:
		bool handleDigShaft(CommandSource source, const DigShaft& event);
		bool handleDigTile(const DigTile& event);
		bool handleAddResource(const AddResource& event);

		ITerrainAlterationService& _terrain;
		IEconomyResourceService& _economy;
		long long _tick{ 0 };
	};

}
