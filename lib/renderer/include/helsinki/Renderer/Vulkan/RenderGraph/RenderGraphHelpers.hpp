#pragma once
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>

namespace hl
{

	class RenderGraphHelpers
	{
		RenderGraphHelpers() = delete;
	public:
		static RenderpassInfo createTextRenderpassInfo(
			const std::string& cameraMatrixResourceId);

		static RenderpassInfo createCompositeRenderpassInfo(
			const std::vector<std::string>& inputs);
	};

}