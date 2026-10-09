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

		static RenderpassInfo createParticleSimPass();
		static PipelineInfo particleDrawPipelineInfo(const std::string& cameraMatrixResourceId);
		static PipelineInfo particleQuadPipelineInfo(const std::string& cameraMatrixResourceId);

		static constexpr const char* ShadowPassName = "shadow_pass";
		static constexpr const char* ShadowPipelineName = "shadow_pipeline";
		static constexpr const char* ShadowUboName = "shadow_ubo";
		static constexpr const char* ShadowDepthName = "shadow_depth";

		static RenderpassInfo createShadowMapPass(
			const std::string& depthName,
			uint32_t mapSize,
			const std::string& shadowUboId = ShadowUboName);
		static PipelineInfo shadowPipelineInfo(const std::string& shadowUboId = ShadowUboName);

		static VertexInputInfo uiVertexInputInfo();
	};

}