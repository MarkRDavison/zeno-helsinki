#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>
#include <vector>

namespace hur
{
	struct SpriteClipFrame
	{
		int frameIndex{ 0 };
		glm::vec2 size{ 0.0f, 0.0f };
	};

	class SpriteClipComponent : public hl::Component
	{
	public:
		std::vector<SpriteClipFrame> frames;
		float secondsPerFrame{ 0.08f };
		float elapsed{ 0.0f };
		int frame{ 0 };
		bool destroyOnComplete{ true };
		bool loop{ false };
	};
}
