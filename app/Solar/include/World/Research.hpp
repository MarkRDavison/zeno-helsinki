#pragma once

#include <vector>

namespace slr
{

	struct WorldData;

	using ResearchAmount = float;

	constexpr ResearchAmount EmpireResearchPerTick = 1000.f;
	constexpr float ExtraActiveResearchPenalty = 1.f;
	constexpr long long MaxActiveResearchProjects = 16;

	constexpr long long ResearchProjectSensorsId = 1;
	constexpr long long ResearchProjectArmourId = 2;
	constexpr long long ResearchProjectDriveId = 3;

	constexpr ResearchAmount ResearchProjectSensorsCost = 5000.f;
	constexpr ResearchAmount ResearchProjectArmourCost = 10000.f;
	constexpr ResearchAmount ResearchProjectDriveCost = 15000.f;

	struct ResearchProjectSpec
	{
		long long id{ 0 };
		const char* name{ "" };
		ResearchAmount cost{ 0.f };
	};

	struct ResearchProgress
	{
		long long empireId{ 0 };
		long long projectId{ 0 };
		ResearchAmount progress{ 0.f };
		bool active{ false };
		bool completed{ false };
	};

	const std::vector<ResearchProjectSpec>& researchCatalogue();
	const ResearchProjectSpec* researchProjectById(long long projectId);
	ResearchAmount researchPointsAddedPerActiveProject(long long activeCount);
	ResearchAmount researchPoolUsedFraction(long long activeCount);
	long long countActiveResearchProjects(const WorldData& world, long long empireId);
	long long researchDisplayedAmount(ResearchAmount amount);
	void applyResearch(WorldData& world);

}
