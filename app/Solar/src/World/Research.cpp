#include <World/Research.hpp>
#include <World/WorldData.hpp>
#include <cmath>

namespace slr
{

	const std::vector<ResearchProjectSpec>& researchCatalogue()
	{
		static const std::vector<ResearchProjectSpec> catalogue{
			ResearchProjectSpec{
				.id = ResearchProjectLaserDamage1Id,
				.name = "Laser damage I",
				.cost = ResearchProjectLevel1Cost },
			ResearchProjectSpec{
				.id = ResearchProjectLaserDamage2Id,
				.name = "Laser damage II",
				.cost = ResearchProjectLevel2Cost,
				.requiresProjectId = ResearchProjectLaserDamage1Id },
			ResearchProjectSpec{
				.id = ResearchProjectLaserDamage3Id,
				.name = "Laser damage III",
				.cost = ResearchProjectLevel3Cost,
				.requiresProjectId = ResearchProjectLaserDamage2Id },
			ResearchProjectSpec{
				.id = ResearchProjectLaserRange1Id,
				.name = "Laser range I",
				.cost = ResearchProjectLevel1Cost },
			ResearchProjectSpec{
				.id = ResearchProjectLaserRange2Id,
				.name = "Laser range II",
				.cost = ResearchProjectLevel2Cost,
				.requiresProjectId = ResearchProjectLaserRange1Id },
			ResearchProjectSpec{
				.id = ResearchProjectLaserRange3Id,
				.name = "Laser range III",
				.cost = ResearchProjectLevel3Cost,
				.requiresProjectId = ResearchProjectLaserRange2Id },
		};
		return catalogue;
	}

	const ResearchProjectSpec* researchProjectById(long long projectId)
	{
		for (const auto& spec : researchCatalogue())
		{
			if (spec.id == projectId)
			{
				return &spec;
			}
		}

		return nullptr;
	}

	ResearchAmount researchPointsAddedPerActiveProject(long long activeCount)
	{
		if (activeCount <= 0)
		{
			return 0.f;
		}

		const float activeCountFloat = static_cast<float>(activeCount);
		return EmpireResearchPerTick /
			(activeCountFloat *
				(1.f + ExtraActiveResearchPenalty * (activeCountFloat - 1.f)));
	}

	ResearchAmount researchPoolUsedFraction(long long activeCount)
	{
		if (activeCount <= 0)
		{
			return 0.f;
		}

		return (static_cast<float>(activeCount) * researchPointsAddedPerActiveProject(activeCount)) /
			EmpireResearchPerTick;
	}

	long long countActiveResearchProjects(const WorldData& world, long long empireId)
	{
		long long count = 0;
		for (const auto& row : world.researchProgress)
		{
			if (row.empireId == empireId && row.active && !row.completed)
			{
				++count;
			}
		}

		return count;
	}

	long long researchDisplayedAmount(ResearchAmount amount)
	{
		if (amount < 0.f)
		{
			return 0;
		}

		return static_cast<long long>(std::floor(amount));
	}

	bool researchProjectPrerequisiteIsMet(
		const WorldData& world,
		long long empireId,
		const ResearchProjectSpec& spec)
	{
		if (spec.requiresProjectId == 0)
		{
			return true;
		}

		const ResearchProgress* required =
			world.researchProgressByEmpireAndProject(empireId, spec.requiresProjectId);
		return required != nullptr && required->completed;
	}

	bool researchProjectIsAvailable(
		const WorldData& world,
		long long empireId,
		const ResearchProjectSpec& spec)
	{
		const ResearchProgress* row = world.researchProgressByEmpireAndProject(empireId, spec.id);
		if (row != nullptr && (row->active || row->completed))
		{
			return false;
		}

		return researchProjectPrerequisiteIsMet(world, empireId, spec);
	}

	void applyCompletedResearch(WorldData& world, long long empireId, long long projectId)
	{
		ResearchProgress* row = world.researchProgressByEmpireAndProject(empireId, projectId);
		if (row == nullptr || !row->completed || row->unlockApplied)
		{
			return;
		}

		Empire* empire = world.empireById(empireId);
		if (empire == nullptr)
		{
			return;
		}

		if (projectId == ResearchProjectLaserDamage1Id)
		{
			empire->stats.laserDamageMultiplier = LaserMultiplierLevel1;
		}
		else if (projectId == ResearchProjectLaserDamage2Id)
		{
			empire->stats.laserDamageMultiplier = LaserMultiplierLevel2;
		}
		else if (projectId == ResearchProjectLaserDamage3Id)
		{
			empire->stats.laserDamageMultiplier = LaserMultiplierLevel3;
		}
		else if (projectId == ResearchProjectLaserRange1Id)
		{
			empire->stats.laserRangeMultiplier = LaserMultiplierLevel1;
		}
		else if (projectId == ResearchProjectLaserRange2Id)
		{
			empire->stats.laserRangeMultiplier = LaserMultiplierLevel2;
		}
		else if (projectId == ResearchProjectLaserRange3Id)
		{
			empire->stats.laserRangeMultiplier = LaserMultiplierLevel3;
		}

		row->unlockApplied = true;
	}

	void applyResearch(WorldData& world)
	{
		for (const auto& empire : world.empires)
		{
			const long long activeCount = countActiveResearchProjects(world, empire.id);
			const ResearchAmount share = researchPointsAddedPerActiveProject(activeCount);
			if (share <= 0.f)
			{
				continue;
			}

			for (auto& row : world.researchProgress)
			{
				if (row.empireId != empire.id || !row.active || row.completed)
				{
					continue;
				}

				const ResearchProjectSpec* spec = researchProjectById(row.projectId);
				if (spec == nullptr)
				{
					continue;
				}

				row.progress += share;
				if (row.progress >= spec->cost)
				{
					row.progress = spec->cost;
					row.active = false;
					row.completed = true;
					applyCompletedResearch(world, empire.id, row.projectId);
				}
			}
		}
	}

}
