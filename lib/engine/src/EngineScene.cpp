
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/System/HelsinkiTracy.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/CameraUniformBufferObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>

namespace hl
{

    static VkExtent2D getExtent(VkExtent2D framebufferExtent, VkExtent2D swapchainExtent)
    {
        if (framebufferExtent.width == 0 || framebufferExtent.height == 0)
        {
            return swapchainExtent;
        }

        return framebufferExtent;
    }

	EngineScene::EngineScene(
        hl::Engine& engine
    ) :
        _engine(engine)
	{

	}
	EngineScene::~EngineScene()
	{
        if (_camera != nullptr)
        {
            delete _camera;
            _camera = nullptr;
        }
	}

	void EngineScene::initialise(
        const std::string& cameraMatrixResourceId,
		VulkanDevice& device, 
		VulkanSwapChain& swapChain,
		VulkanCommandPool& graphicsCommandPool,
        VulkanCommandPool& /*transferCommandPool*/,
		ResourceManager& resourceManager,
        MaterialSystem& materialSystem,
		const std::vector<RenderpassInfo>& renderpassInfo)
	{
        _device = &device;
        _swapChain = &swapChain;
        _resourceManager = &resourceManager;
        _materialSystem = &materialSystem;

        _cameraMatrixPushConstantHandle = ResourceHandle<UniformBufferResource>(cameraMatrixResourceId, _resourceManager);

		_renderGraph = new hl::GeneratedRenderGraph(
			device,
			swapChain,
			renderpassInfo,
			resourceManager);

		_frameResources.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo primaryAllocInfo{};
		primaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		primaryAllocInfo.commandPool = graphicsCommandPool._commandPool;
		primaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		primaryAllocInfo.commandBufferCount = (uint32_t)_frameResources.size();

		std::vector<VkCommandBuffer> perFrameCommandBuffers(MAX_FRAMES_IN_FLIGHT);

		CHECK_VK_RESULT(vkAllocateCommandBuffers(device._device, &primaryAllocInfo, perFrameCommandBuffers.data()));

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			auto& frame = _frameResources[i];
			frame.primaryCmd = perFrameCommandBuffers[i];

			device.setDebugName(
				reinterpret_cast<uint64_t>(frame.primaryCmd),
				VK_OBJECT_TYPE_COMMAND_BUFFER,
				("PrimaryCommandBuffer_" + std::to_string(i)).c_str());

			for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); layer++)
			{
				const auto& renderpassesForLayer = _renderGraph->getSortedNodesByNameForLayer(layer);
				auto& secondaryCommandsForLayerAndGroups = frame.secondaryCommandsByLayerAndPipelineGroup[layer];

				secondaryCommandsForLayerAndGroups.resize(renderpassesForLayer.size());

				for (size_t rpIndex = 0; rpIndex < renderpassesForLayer.size(); ++rpIndex)
				{
					const auto& renderpass = _renderGraph->getRenderpassByName(renderpassesForLayer[rpIndex]);
					auto& secondaryCommandsForGroups = secondaryCommandsForLayerAndGroups[rpIndex];

					secondaryCommandsForGroups.resize(renderpass->getPipelineGroups().size());

					uint32_t groupIndex = 0;
					for (auto& secondaryCommand : secondaryCommandsForGroups)
					{
						VkCommandBufferAllocateInfo secondaryAllocInfo{};
						secondaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
						secondaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
						secondaryAllocInfo.commandPool = graphicsCommandPool._commandPool;
						secondaryAllocInfo.commandBufferCount = 1;

						vkAllocateCommandBuffers(device._device, &secondaryAllocInfo, &secondaryCommand);

						device.setDebugName(
							reinterpret_cast<uint64_t>(secondaryCommand),
							VK_OBJECT_TYPE_COMMAND_BUFFER,
							("SecondaryCommandBuffer_" + std::to_string(i) + "_" + renderpass->Name + "_Group_" + std::to_string(groupIndex)).c_str());

						groupIndex++;
					}
				}
			}
		}
	}
	void EngineScene::initialise(
        const std::string& cameraMatrixResourceId,
		VulkanDevice& device,
		VulkanSwapChain& swapChain,
        VulkanCommandPool& graphicsCommandPool,
        VulkanCommandPool& transferCommandPool,
		ResourceManager& resourceManager,
        MaterialSystem& materialSystem)
	{
		initialise(cameraMatrixResourceId, device, swapChain, graphicsCommandPool, transferCommandPool, resourceManager, materialSystem, {});
	}
	void EngineScene::cleanup()
	{
		_renderGraph->destroy();
		delete _renderGraph;
		_renderGraph = nullptr;
	}

    void EngineScene::updateBase(uint32_t currentFrame, float delta)
    {
        ZoneScopedN("Engine Scene Update");
        update(currentFrame, delta);
        updateCameraUniformBuffer(_cameraMatrixPushConstantHandle.Get()->getUniformBuffer(currentFrame));
    }
	void EngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
	}
	VkCommandBuffer EngineScene::draw(uint32_t currentFrame, uint32_t imageIndex)
	{
		ZoneScopedN("Engine Scene Draw");

        const auto& lastRenderpassName = _renderGraph->getResources().back()->Name;
        auto& frame = _frameResources[currentFrame];

        CHECK_VK_RESULT(vkResetCommandBuffer(frame.primaryCmd, 0));
        for (auto& [_, secondaries] : frame.secondaryCommandsByLayerAndPipelineGroup)
        {
            for (auto& secondaryGroup : secondaries)
            {
                for (auto& secondary : secondaryGroup)
                {
                    CHECK_VK_RESULT(vkResetCommandBuffer(secondary, 0));
                }
            }
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        CHECK_VK_RESULT(vkBeginCommandBuffer(frame.primaryCmd, &beginInfo));

        for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); ++layer)
        {
            ZoneScoped;
            ZoneNameF("record command buffers for layer %s", std::to_string(layer).c_str());

            assert(frame.secondaryCommandsByLayerAndPipelineGroup.at(layer).size() == _renderGraph->getSortedNodesByNameForLayer(layer).size());

            const auto& secondaryBuffersPerRenderpass = frame.secondaryCommandsByLayerAndPipelineGroup.at(layer);

            // FUTURE WORK: Multithreaded secondary command buffer recording
            //
            // Current design records all pipeline groups in each renderpass sequentially.
            // Potential optimization: record pipeline groups in parallel on worker threads.
            //
            // Plan:
            // 1. For each renderpass, if it has >1 pipeline group, dispatch each group to a worker thread.
            // 2. Each thread records its secondary command buffer independently.
            // 3. Wait for all threads to finish.
            // 4. Accumulate all secondary command buffers and call vkCmdExecuteCommands once to submit them to the primary command buffer.
            //
            // Notes:
            // - Only safe for independent pipeline groups within a renderpass (no ordering dependencies).
            //      - This should be fine since renderpasses in the same layer should be independant
            // - Avoid multithreading overhead if there is only one pipeline group.
            // - Later extension: consider parallel renderpasses if layers or renderpasses are independent.
            size_t renderpassIndex = 0;
            for (const auto& renderpassName : _renderGraph->getSortedNodesByNameForLayer(layer))
            {
                ZoneScoped;
                ZoneNameF("record command buffer for %s", renderpassName.c_str());

                const auto& renderpass = _renderGraph->getRenderpassByName(renderpassName);
                const auto& clearValues = renderpass->getClearValues();
                const auto& framebuffer = renderpass->getFramebuffer(lastRenderpassName == renderpass->Name ? imageIndex : currentFrame);

                VkRenderPassBeginInfo renderpassBegin
                {
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = renderpass->getRenderPass(),
                    .framebuffer = framebuffer,
                    .renderArea =
                    {
                        .offset = { 0,0 },
                        .extent = getExtent(renderpass->getExtent(), _swapChain->_swapChainExtent),
                    },
                    .clearValueCount = (uint32_t)clearValues.size(),
                    .pClearValues = clearValues.data()
                };

                vkCmdBeginRenderPass(frame.primaryCmd, &renderpassBegin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass->getRenderPass();
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = framebuffer;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;

                VkCommandBufferBeginInfo secondaryBeginInfo{};
                secondaryBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                secondaryBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                secondaryBeginInfo.pInheritanceInfo = &inheritanceInfo;

                const auto& secondaryBuffersForGroup = secondaryBuffersPerRenderpass[renderpassIndex];

                size_t pipelineGroupIndex = 0;
                for (const auto& pg : renderpass->getPipelineGroups())
                {
                    // TODO: Here create a job and put it into a worker thread queue to be picked up
                    const auto secondaryBuffer = secondaryBuffersForGroup[pipelineGroupIndex];

                    CHECK_VK_RESULT(vkBeginCommandBuffer(secondaryBuffer, &secondaryBeginInfo));

                    for (const auto& p : pg)
                    {
                        vkCmdBindPipeline(secondaryBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p->getPipeline());
                        {   //  TODO: This can be wasteful if nothing has changed.
                            VkViewport viewport
                            {
                                .x = 0.0f,
                                .y = 0.0f,
                                .width = (float)getExtent(renderpass->getExtent(), _swapChain->_swapChainExtent).width,
                                .height = (float)getExtent(renderpass->getExtent(), _swapChain->_swapChainExtent).height,
                                .minDepth = 0.0f,
                                .maxDepth = 1.0f
                            };
                            vkCmdSetViewport(secondaryBuffer, 0, 1, &viewport);

                            VkRect2D scissor
                            {
                                .offset = { 0, 0 } ,
                                .extent = getExtent(renderpass->getExtent(), _swapChain->_swapChainExtent)
                            };
                            vkCmdSetScissor(secondaryBuffer, 0, 1, &scissor);
                        }

                        renderPipelineDraw(secondaryBuffer, p, currentFrame);
                    }

                    CHECK_VK_RESULT(vkEndCommandBuffer(secondaryBuffer));

                    // TODO: Join/wait for all the thread jobs to complete here
                    // TODO: Accumulate all the secondary buffers and call vkCmdExecuteCommands just before vkCmdEndRenderPass
                    vkCmdExecuteCommands(frame.primaryCmd, 1, &secondaryBuffer);

                    pipelineGroupIndex++;
                }

                vkCmdEndRenderPass(frame.primaryCmd);

                ++renderpassIndex;
            }
        }

        CHECK_VK_RESULT(vkEndCommandBuffer(frame.primaryCmd));

        return frame.primaryCmd;
	}

	void EngineScene::recreate(uint32_t width, uint32_t height)
	{
		_renderGraph->recreate((uint32_t)width, (uint32_t)height);
	}
	void EngineScene::updateAllDescriptorSets()
	{
		_renderGraph->updateAllDescriptorSets();
	}
    void EngineScene::updateAllOutputResources()
    {
        _renderGraph->updateAllOutputResources();
    }
    void EngineScene::updateCameraUniformBuffer(VulkanUniformBuffer& uniformBuffer)
    {
        CameraUniformBufferObject ubo{};
        ((Camera*)_camera)->setAspectRatio(_swapChain->_swapChainExtent.width / (float)_swapChain->_swapChainExtent.height);
        ubo.view = _camera->getViewMatrix();
        ubo.proj = _camera->getProjectionMatrix();

        ubo.proj[1][1] *= -1;

        uniformBuffer.writeToBuffer(&ubo);
    }
    void EngineScene::renderPipelineDraw(VkCommandBuffer commandBuffer, hl::VulkanRenderGraphPipelineResources* pipeline, uint32_t currentFrame)
    {
        // TODO: Need a system to register draw commands for different pipelines.
        if (pipeline->Name == "skybox_pipeline")
        {
            auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->getPipelineLayout(),
                0,
                1,
                &descriptorSet,
                0,
                nullptr);

            vkCmdDraw(commandBuffer, 36, 1, 0, 0); // quad from 12 triangles
        }
        else if (pipeline->Name == "model_pipeline")
        {
            for (const auto& entity : _scene.getEntities())
            {
                const auto& transform = entity->GetComponent<hl::TransformComponent>();
                const auto& model = entity->GetComponent<hl::ModelComponent>();

                const auto& modelResource = _resourceManager->GetResource<hl::ModelResource>(model->getModelId());
                //const auto& modelResource = _resourceManager->HasResource<hl::ModelResource>(model->getModelId())
                //    ? _resourceManager->GetResource<hl::ModelResource>(model->getModelId())
                //    : _resourceManager->GetResource<hl::ModelResource>("FALLBACK_MODEL");

                auto modelTransform = transform->GetTransformMatrix();
                const auto& meshes = modelResource->getMeshes();

                auto pc = hl::MaterialPushConstantObject
                {
                    .model = modelTransform
                };

                for (const auto& mesh : meshes)
                {
                    pc.materialIndex = _materialSystem->getMaterialIndex(mesh.materialName);
                    // TODO: Somehow need to know if this material isn't loaded, and use a fallback material index, 
                    // When I use textures as part of materials...

                    vkCmdPushConstants(
                        commandBuffer,
                        pipeline->getPipelineLayout(),
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(hl::MaterialPushConstantObject),
                        &pc
                    );

                    VkBuffer vertexBuffers[] = { mesh._vertexBuffer._buffer };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(
                        commandBuffer,
                        0,
                        1,
                        vertexBuffers,
                        offsets);

                    vkCmdBindIndexBuffer(
                        commandBuffer,
                        mesh._indexBuffer._buffer,
                        0,
                        VK_INDEX_TYPE_UINT32);

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

                    vkCmdDrawIndexed(commandBuffer, mesh._indexCount, 1, 0, 0, 0);
                }
            }
        }
        else if (pipeline->Name == "postprocess_pipeline")
        {
            auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->getPipelineLayout(),
                0,
                1,
                &descriptorSet,
                0,
                nullptr);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
        }
        else if (pipeline->Name == "fullscreen_sample")
        {
            auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->getPipelineLayout(),
                0,
                1,
                &descriptorSet,
                0,
                nullptr);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // fullscreen triangle
        }
        else if (pipeline->Name == "ui")
        {
            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // halfscreen triangle
        }
        else if (pipeline->Name == "composite_pipeline")
        {
            auto descriptorSet = pipeline->getDescriptorSet(currentFrame);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->getPipelineLayout(),
                0,
                1,
                &descriptorSet,
                0,
                nullptr);
            vkCmdDraw(commandBuffer, 3, 1, 0, 0); // halfscreen triangle
        }
        else if (pipeline->Name == "sprite_pipeline")
        {
            for (const auto& entity : _scene.getEntities())
            {
                const auto& transform = entity->GetComponent<hl::TransformComponent>();
                const auto& sprite = entity->GetComponent<hl::SpriteComponent>();

                auto modelTransform = transform->GetTransformMatrix();

                auto pc = hl::SpritePushConstantObject
                {
                    .model = modelTransform,
                    .size = glm::vec2(64.0f, 64.0f),
                    .frameIndex = sprite->getFrameDataIndex()
                };

                vkCmdPushConstants(
                    commandBuffer,
                    pipeline->getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(hl::SpritePushConstantObject),
                    &pc
                );

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

                vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            }
        }
        else if (pipeline->Name == "text_pipeline")
        {
            int cellIndex = 4;
            vkCmdPushConstants(
                commandBuffer,
                pipeline->getPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(int),
                &cellIndex
            );

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

            vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        }
        else
        {
            throw std::runtime_error("TODO: HARD CODED DRAW FUNCTIONS");
        }
    }
}