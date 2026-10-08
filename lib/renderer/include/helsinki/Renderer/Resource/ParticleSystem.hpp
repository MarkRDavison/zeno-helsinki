#pragma once

#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/System/glm.hpp>
#include <cstdint>

namespace hl
{
	class VulkanRenderGraphPipelineResources;

	enum class ParticleSpace : uint32_t
	{
		World3D = 0,
		Ortho2D = 1
	};

	struct Particle
	{
		glm::vec3 position{ 0.0f };
		float life{ 0.0f };
		glm::vec3 velocity{ 0.0f };
		float pad{ 0.0f };
	};

	struct ParticleEmitterUniformBufferObject
	{
		glm::vec4 origin{ 0.0f };
		glm::vec4 gravity{ 0.0f };
		glm::uvec4 counts{ 0 };
		glm::uvec4 limits{ 0 };
	};

	class ParticleSystem
	{
	public:
		static constexpr const char StorageBufferName[] = "particle_ssbo";
		static constexpr const char EmitterUniformBufferName[] = "particle_emitter_ubo";
		static constexpr const char SimPassName[] = "particle_sim_pass";
		static constexpr const char ComputePipelineName[] = "particle_sim_pipeline";
		static constexpr const char DrawPipelineName[] = "particle_pipeline";
		static constexpr const char QuadPipelineName[] = "particle_quad_pipeline";

		ParticleSystem(VulkanDevice& device, ResourceManager& resourceManager);

		void create();
		void destroy();

		void setEmitter(ParticleSpace space, uint32_t cameraIndex, const glm::vec3& origin);
		void setSimSteps(uint32_t stepCount);
		void updateGpuResources(uint32_t currentFrame);

		void recordCompute(
			VkCommandBuffer commandBuffer,
			VulkanRenderGraphPipelineResources* pipeline,
			uint32_t currentFrame);
		void recordDraw(
			VkCommandBuffer commandBuffer,
			VulkanRenderGraphPipelineResources* pipeline,
			uint32_t currentFrame);
		void recordDrawQuads(
			VkCommandBuffer commandBuffer,
			VulkanRenderGraphPipelineResources* pipeline,
			uint32_t currentFrame);

		uint32_t maxParticles() const { return MAX_PARTICLES; }

	private:
		VulkanDevice& _device;
		ResourceManager& _resourceManager;
		hl::ResourceHandle<hl::StorageBufferResource> _particleStorageHandle;
		hl::ResourceHandle<hl::UniformBufferResource> _emitterUniformHandle;
		ParticleSpace _space{ ParticleSpace::World3D };
		uint32_t _cameraIndex{ 0 };
		glm::vec3 _origin{ 0.0f };
		glm::vec3 _gravity{ 0.0f, -1.2f, 0.0f };
		uint32_t _simSteps{ 0 };
	};
}
