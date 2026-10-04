#include <Systems/SpriteClipSystem.hpp>
#include <Components/SpriteClipComponent.hpp>
#include <Components/EntityComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <algorithm>

namespace hur
{
	namespace
	{
		void applyClipFrame(hl::SpriteComponent& sprite, EntityComponent& ec, const SpriteClipComponent& clip)
		{
			if (clip.frames.empty())
			{
				return;
			}

			const auto index = std::clamp(clip.frame, 0, static_cast<int>(clip.frames.size()) - 1);
			const auto& current = clip.frames[static_cast<std::size_t>(index)];
			sprite.setFrameDataIndex(current.frameIndex);
			ec.Size = current.size;
		}
	}

	SpriteClipSystem::SpriteClipSystem(hl::Scene& scene) :
		_scene(scene)
	{
	}

	void SpriteClipSystem::update(float delta)
	{
		for (auto* entity : _scene.getEntitiesWithComponents<SpriteClipComponent, hl::SpriteComponent, EntityComponent>())
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* clip = entity->GetComponent<SpriteClipComponent>();
			auto* sprite = entity->GetComponent<hl::SpriteComponent>();
			auto* ec = entity->GetComponent<EntityComponent>();

			if (clip->frames.empty())
			{
				if (clip->destroyOnComplete)
				{
					_scene.removeEntity(entity->Id);
				}
				continue;
			}

			applyClipFrame(*sprite, *ec, *clip);

			if (clip->secondsPerFrame <= 0.0f)
			{
				continue;
			}

			clip->elapsed += delta;
			while (clip->elapsed >= clip->secondsPerFrame)
			{
				clip->elapsed -= clip->secondsPerFrame;
				++clip->frame;

				if (clip->frame >= static_cast<int>(clip->frames.size()))
				{
					if (clip->loop)
					{
						clip->frame = 0;
						applyClipFrame(*sprite, *ec, *clip);
						continue;
					}

					if (clip->destroyOnComplete)
					{
						_scene.removeEntity(entity->Id);
					}
					else
					{
						clip->frame = static_cast<int>(clip->frames.size()) - 1;
						applyClipFrame(*sprite, *ec, *clip);
					}
					break;
				}

				applyClipFrame(*sprite, *ec, *clip);
			}
		}
	}
}
