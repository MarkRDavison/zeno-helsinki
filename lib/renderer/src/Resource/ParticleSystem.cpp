#include <helsinki/Renderer/Resource/ParticleSystem.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/System/HelsinkiTracy.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <vulkan/vulkan.h>
#include <cassert>

namespace hl
{

	ParticleSystem::ParticleSystem(
		VulkanDevice& device,
		ResourceManager& resourceManager
	) :
		_device(device),
		_resourceManager(resourceManager)
	{

	}

	void ParticleSystem::create()
	{
		auto context = ResourceContext
		{
			.device = &_device,
			.pool = nullptr,
			.resourceManager = &_resourceManager,
			.rootPath = ""
		};

		_particleStorageHandle = _resourceManager.Load<hl::StorageBufferResource>(
			StorageBufferName,
			context,
			sizeof(Particle),
			static_cast<uint32_t>(MAX_PARTICLES * MAX_FRAMES_IN_FLIGHT));

		Particle dead{};
		for (uint32_t i = 0; i < static_cast<uint32_t>(MAX_PARTICLES * MAX_FRAMES_IN_FLIGHT); ++i)
		{
			_particleStorageHandle.Get()->writeToBuffer(&dead, i);
		}

		_emitterUniformHandle = _resourceManager.Load<hl::UniformBufferResource>(
			EmitterUniformBufferName,
			context,
			sizeof(ParticleEmitterUniformBufferObject),
			MAX_FRAMES_IN_FLIGHT,
			1);
	}

	void ParticleSystem::destroy()
	{
		if (_emitterUniformHandle.IsValid())
		{
			_resourceManager.Release(_emitterUniformHandle.GetId());
			_emitterUniformHandle = {};
		}
		if (_particleStorageHandle.IsValid())
		{
			_resourceManager.Release(_particleStorageHandle.GetId());
			_particleStorageHandle = {};
		}
	}

	void ParticleSystem::setEmitter(ParticleSpace space, uint32_t cameraIndex, const glm::vec3& origin)
	{
		_space = space;
		_cameraIndex = cameraIndex;
		_origin = origin;
		if (space == ParticleSpace::Ortho2D)
		{
			_gravity = glm::vec3(0.0f, 180.0f, 0.0f);
		}
		else
		{
			_gravity = glm::vec3(0.0f, -1.2f, 0.0f);
		}
	}

	void ParticleSystem::setSimSteps(uint32_t stepCount)
	{
		_simSteps = stepCount;
	}

	void ParticleSystem::updateGpuResources(uint32_t currentFrame)
	{
		if (!_emitterUniformHandle.IsValid())
		{
			return;
		}

		ParticleEmitterUniformBufferObject ubo{};
		ubo.origin = glm::vec4(_origin, 0.0f);
		ubo.gravity = glm::vec4(_gravity, 0.0f);
		ubo.counts = glm::uvec4(
			_simSteps,
			currentFrame,
			static_cast<uint32_t>(_space),
			_cameraIndex);
		ubo.limits = glm::uvec4(0, static_cast<uint32_t>(MAX_PARTICLES), 0, 0);
		_emitterUniformHandle.Get()->getUniformBuffer(currentFrame).writeToBuffer(&ubo, 0);
	}

	void ParticleSystem::recordCompute(
		VkCommandBuffer commandBuffer,
		VulkanRenderGraphPipelineResources* pipeline,
		uint32_t currentFrame)
	{
		ZoneScopedN("particle sim");
		assert(pipeline != nullptr);
		assert(commandBuffer != VK_NULL_HANDLE);

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline->getPipeline());

		auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		const uint32_t groups = (static_cast<uint32_t>(MAX_PARTICLES) + 63u) / 64u;
		vkCmdDispatch(commandBuffer, groups, 1, 1);
	}

	void ParticleSystem::recordDraw(
		VkCommandBuffer commandBuffer,
		VulkanRenderGraphPipelineResources* pipeline,
		uint32_t currentFrame)
	{
		ZoneScopedN("particle draw");
		assert(pipeline != nullptr);
		assert(commandBuffer != VK_NULL_HANDLE);

		auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(MAX_PARTICLES), 1, 0, 0);
	}

	void ParticleSystem::recordDrawQuads(
		VkCommandBuffer commandBuffer,
		VulkanRenderGraphPipelineResources* pipeline,
		uint32_t currentFrame)
	{
		ZoneScopedN("particle quad draw");
		assert(pipeline != nullptr);
		assert(commandBuffer != VK_NULL_HANDLE);

		auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(MAX_PARTICLES) * 6u, 1, 0, 0);
	}
}
