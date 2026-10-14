#pragma once

#include <vector>

namespace slr
{

	struct WorldData;

	using ResearchAmount = float;

	constexpr ResearchAmount EmpireResearchPerTick = 1000.f;
	constexpr float ExtraActiveResearchPenalty = 1.f;
	constexpr long long MaxActiveResearchProjects = 16;

	constexpr long long ResearchProjectLaserDamage1Id = 1;
	constexpr long long ResearchProjectLaserDamage2Id = 2;
	constexpr long long ResearchProjectLaserDamage3Id = 3;
	constexpr long long ResearchProjectLaserRange1Id = 4;
	constexpr long long ResearchProjectLaserRange2Id = 5;
	constexpr long long ResearchProjectLaserRange3Id = 6;

	constexpr ResearchAmount ResearchProjectLevel1Cost = 5000.f;
	constexpr ResearchAmount ResearchProjectLevel2Cost = 10000.f;
	constexpr ResearchAmount ResearchProjectLevel3Cost = 15000.f;

	constexpr float LaserMultiplierLevel1 = 1.25f;
	constexpr float LaserMultiplierLevel2 = 1.5f;
	constexpr float LaserMultiplierLevel3 = 2.0f;

	struct ResearchProjectSpec
	{
		long long id{ 0 };
		const char* name{ "" };
		ResearchAmount cost{ 0.f };
		long long requiresProjectId{ 0 };
	};

	struct ResearchProgress
	{
		long long empireId{ 0 };
		long long projectId{ 0 };
		ResearchAmount progress{ 0.f };
		bool active{ false };
		bool completed{ false };
		bool unlockApplied{ false };
	};

	const std::vector<ResearchProjectSpec>& researchCatalogue();
	const ResearchProjectSpec* researchProjectById(long long projectId);
	ResearchAmount researchPointsAddedPerActiveProject(long long activeCount);
	ResearchAmount researchPoolUsedFraction(long long activeCount);
	long long countActiveResearchProjects(const WorldData& world, long long empireId);
	long long researchDisplayedAmount(ResearchAmount amount);
	bool researchProjectPrerequisiteIsMet(
		const WorldData& world,
		long long empireId,
		const ResearchProjectSpec& spec);
	bool researchProjectIsAvailable(
		const WorldData& world,
		long long empireId,
		const ResearchProjectSpec& spec);
	void applyCompletedResearch(WorldData& world, long long empireId, long long projectId);
	void applyResearch(WorldData& world);

}
