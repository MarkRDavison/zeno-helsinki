#pragma once

#include <Combat.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/StatusTypes.hpp>
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace tower
{
	struct StatusInstance
	{
		std::string defId;
		float remaining = 0.0f;
		float tickElapsed = 0.0f;
		int seq = 0;
	};

	enum class ApplyStatusOutcome
	{
		Applied,
		Ignored
	};

	struct DotTick
	{
		float damage = 0.0f;
		std::string damageType;
	};

	struct StatusRing
	{
		std::string categoryId;
		glm::vec3 color{ 1.0f };
		float scaleXZ = 1.0f;
	};

	inline constexpr float kStatusRingBaseScale = 1.15f;
	inline constexpr float kStatusRingScaleStep = 0.22f;

	inline const StatusCategoryDef* categoryOf(
		const StatusInstance& instance,
		const StatusCatalog& statuses,
		const StatusCategoryCatalog& categories)
	{
		const auto* def = statuses.find(instance.defId);
		if (def == nullptr)
		{
			return nullptr;
		}

		return categories.find(def->category);
	}

	inline ApplyStatusOutcome applyStatus(
		std::vector<StatusInstance>& list,
		const StatusDef& def,
		const StatusCategoryDef& category,
		const StatusCatalog& statuses,
		int& nextSeq)
	{
		int defCount = 0;
		int catCount = 0;
		for (const auto& instance : list)
		{
			if (instance.defId == def.id)
			{
				++defCount;
			}

			const auto* other = statuses.find(instance.defId);
			if (other != nullptr && other->category == category.id)
			{
				++catCount;
			}
		}

		const bool defFull = defCount >= def.cap;
		const bool catFull = catCount >= category.cap;
		if (!defFull && !catFull)
		{
			list.push_back(StatusInstance{
				.defId = def.id,
				.remaining = def.duration,
				.tickElapsed = 0.0f,
				.seq = nextSeq++
			});
			return ApplyStatusOutcome::Applied;
		}

		std::vector<std::size_t> candidates;
		for (std::size_t i = 0; i < list.size(); ++i)
		{
			const auto* other = statuses.find(list[i].defId);
			if (other == nullptr)
			{
				continue;
			}

			const bool inDef = list[i].defId == def.id;
			const bool inCat = other->category == category.id;
			if (defFull && catFull)
			{
				if (inDef)
				{
					candidates.push_back(i);
				}
			}
			else if (defFull)
			{
				if (inDef)
				{
					candidates.push_back(i);
				}
			}
			else if (inCat)
			{
				candidates.push_back(i);
			}
		}

		if (candidates.empty())
		{
			return ApplyStatusOutcome::Ignored;
		}

		bool worseThanAll = true;
		for (const auto index : candidates)
		{
			const auto* other = statuses.find(list[index].defId);
			const int score = other != nullptr ? other->score : 0;
			if (def.score >= score)
			{
				worseThanAll = false;
				break;
			}
		}

		if (worseThanAll)
		{
			return ApplyStatusOutcome::Ignored;
		}

		std::size_t evict = candidates.front();
		for (const auto index : candidates)
		{
			const auto* evictDef = statuses.find(list[evict].defId);
			const auto* candDef = statuses.find(list[index].defId);
			const int evictScore = evictDef != nullptr ? evictDef->score : 0;
			const int candScore = candDef != nullptr ? candDef->score : 0;
			if (candScore < evictScore)
			{
				evict = index;
				continue;
			}

			if (candScore > evictScore)
			{
				continue;
			}

			if (list[index].remaining < list[evict].remaining)
			{
				evict = index;
				continue;
			}

			if (list[index].remaining > list[evict].remaining)
			{
				continue;
			}

			if (list[index].seq < list[evict].seq)
			{
				evict = index;
			}
		}

		list[evict] = StatusInstance{
			.defId = def.id,
			.remaining = def.duration,
			.tickElapsed = 0.0f,
			.seq = nextSeq++
		};
		return ApplyStatusOutcome::Applied;
	}

	inline void tickStatusDurations(std::vector<StatusInstance>& list, float delta)
	{
		for (auto& instance : list)
		{
			instance.remaining -= delta;
		}

		std::erase_if(list, [](const StatusInstance& instance)
		{
			return instance.remaining <= 0.0f;
		});
	}

	inline std::vector<DotTick> tickDots(
		std::vector<StatusInstance>& list,
		const StatusCatalog& statuses,
		float delta)
	{
		std::vector<DotTick> ticks;
		for (auto& instance : list)
		{
			instance.remaining -= delta;
			if (instance.remaining <= 0.0f)
			{
				continue;
			}

			const auto* def = statuses.find(instance.defId);
			if (def == nullptr || def->kind != StatusKind::Dot)
			{
				continue;
			}

			instance.tickElapsed += delta;
			while (instance.remaining > 0.0f && instance.tickElapsed >= def->interval)
			{
				instance.tickElapsed -= def->interval;
				ticks.push_back(DotTick{ .damage = def->tickDamage, .damageType = def->damageType });
			}
		}

		std::erase_if(list, [](const StatusInstance& instance)
		{
			return instance.remaining <= 0.0f;
		});
		return ticks;
	}

	inline float channelSum(
		const std::vector<StatusInstance>& list,
		const StatusCatalog& statuses,
		std::string_view channel)
	{
		float sum = 0.0f;
		for (const auto& instance : list)
		{
			const auto* def = statuses.find(instance.defId);
			if (def != nullptr && def->kind == StatusKind::Stat && def->channel == channel)
			{
				sum += def->magnitude;
			}
		}

		if (isUnitChannel(channel))
		{
			return clampUnit(sum).value;
		}

		return sum;
	}

	inline std::vector<StatusRing> statusRings(
		const std::vector<StatusInstance>& list,
		const StatusCatalog& statuses,
		const StatusCategoryCatalog& categories,
		float creepScale)
	{
		struct CategoryAge
		{
			std::string id;
			int minSeq = 0;
			glm::vec3 color{ 1.0f };
		};

		std::vector<CategoryAge> ages;
		for (const auto& instance : list)
		{
			const auto* category = categoryOf(instance, statuses, categories);
			if (category == nullptr)
			{
				continue;
			}

			auto it = std::find_if(ages.begin(), ages.end(), [&](const CategoryAge& age)
			{
				return age.id == category->id;
			});
			if (it == ages.end())
			{
				ages.push_back(CategoryAge{
					.id = category->id,
					.minSeq = instance.seq,
					.color = category->color
				});
			}
			else if (instance.seq < it->minSeq)
			{
				it->minSeq = instance.seq;
			}
		}

		std::sort(ages.begin(), ages.end(), [](const CategoryAge& a, const CategoryAge& b)
		{
			return a.minSeq < b.minSeq;
		});

		std::vector<StatusRing> rings;
		rings.reserve(ages.size());
		for (std::size_t i = 0; i < ages.size(); ++i)
		{
			rings.push_back(StatusRing{
				.categoryId = ages[i].id,
				.color = ages[i].color,
				.scaleXZ = creepScale * (kStatusRingBaseScale + static_cast<float>(i) * kStatusRingScaleStep)
			});
		}

		return rings;
	}
}
