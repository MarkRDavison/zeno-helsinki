#pragma once

#include <World/WorldData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/glm.hpp>
#include <vector>

namespace hl
{
	class ResourceManager;
	class TextSystem;
}

namespace slr
{

	class GalaxyView
	{
	public:
		void initialise(hl::TextSystem& textSystem, const WorldData& world, int cameraIndex);
		void drawGraph(hl::PipelineDrawData& pdd, hl::ResourceManager& resourceManager) const;
		void drawLabels(hl::PipelineDrawData& pdd, hl::TextSystem& textSystem) const;

	private:
		struct Label
		{
			int textId{ -1 };
			glm::vec2 worldCenter{ 0.0f };
		};

		const WorldData* _world{ nullptr };
		int _cameraIndex{ 0 };
		std::vector<Label> _labels;
	};

}
