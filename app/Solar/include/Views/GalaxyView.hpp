#pragma once

#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/glm.hpp>

namespace hl
{
	class TextSystem;
}

namespace slr
{

	class GalaxyView
	{
	public:
		void initialise(hl::TextSystem& textSystem, int width, int height);
		void layout(hl::TextSystem& textSystem, int width, int height);
		void draw(hl::PipelineDrawData& pdd, hl::TextSystem& textSystem) const;

	private:
		int _labelId{ -1 };
		glm::vec3 _position{ 0.0f };
	};

}
