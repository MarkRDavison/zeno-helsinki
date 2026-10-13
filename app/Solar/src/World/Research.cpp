#include <World/Research.hpp>
#include <World/WorldData.hpp>
#include <cmath>

namespace slr
{

	const std::vector<ResearchProjectSpec>& researchCatalogue()
	{
		static const std::vector<ResearchProjectSpec> catalogue{
			ResearchProjectSpec{
				.id = ResearchProjectSensorsId,
				.name = "Improved sensors",
				.cost = ResearchProjectSensorsCost },
			ResearchProjectSpec{
				.id = ResearchProjectArmourId,
				.name = "Better armour",
				.cost = ResearchProjectArmourCost },
			ResearchProjectSpec{
				.id = ResearchProjectDriveId,
				.name = "Dummy drive",
				.cost = ResearchProjectDriveCost },
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
				}
			}
		}
	}

}
