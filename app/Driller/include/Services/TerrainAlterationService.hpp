#pragma once

#include <Entities/Data/TerrainData.hpp>

namespace drl
{

	class ITerrainAlterationService
	{
	public:
		virtual ~ITerrainAlterationService() = 0;

		virtual bool digShaft(int level) = 0;
		virtual bool digTile(int level, int column) = 0;
		virtual bool createTile(int levelMin, int levelMax, int columnMin, int columnMax) = 0;
		virtual bool doesTileExist(int level, int column) const = 0;
		virtual Tile& getTile(int level, int column) = 0;
		virtual const Tile& getTile(int level, int column) const = 0;
		virtual bool isTileDugOut(int level, int column) const = 0;
		virtual void initialiseTile(int level, int column) = 0;
		virtual bool doesLevelExist(int level) const = 0;
		virtual bool canTileBeReached(int level, int column) const = 0;
		virtual bool isLevelNextShaftLevel(int level) const = 0;
	};

	inline ITerrainAlterationService::~ITerrainAlterationService() = default;

	class TerrainAlterationService : public ITerrainAlterationService
	{
	public:
		explicit TerrainAlterationService(TerrainData& terrainData);
		~TerrainAlterationService() override = default;

		bool digShaft(int level) override;
		bool digTile(int level, int column) override;
		bool createTile(int levelMin, int levelMax, int columnMin, int columnMax) override;
		bool doesTileExist(int level, int column) const override;
		Tile& getTile(int level, int column) override;
		const Tile& getTile(int level, int column) const override;
		bool isTileDugOut(int level, int column) const override;
		void initialiseTile(int level, int column) override;
		bool doesLevelExist(int level) const override;
		bool canTileBeReached(int level, int column) const override;
		bool isLevelNextShaftLevel(int level) const override;

	private:
		TerrainData& _terrainData;
	};

}
