#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>

namespace hl
{

	class SpriteComponent : public Component
	{
	public:
		void setFrameDataIndex(int index) { _frameDataIndex = index; }
		int getFrameDataIndex() const { return _frameDataIndex; }

	private:
		int _frameDataIndex{ 0 };
	};

}