
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/System/HelsinkiTracy.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/CameraUniformBufferObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/ParticleSystem.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <algorithm>
#include <cassert>
#include <future>
#include <stdexcept>

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
        for (auto& [name, camera] : _cameras)
        {
            delete camera;
        }

        _cameras.clear();
	}

	void EngineScene::initialise(
        const std::string& cameraMatrixResourceId,
		VulkanDevice& device, 
		VulkanSwapChain& swapChain,
		VulkanCommandPool& graphicsCommandPool,
        VulkanCommandPool& /*transferCommandPool*/,
		ResourceManager& resourceManager,
		const std::vector<RenderpassInfo>& renderpassInfo)
	{
        _device = &device;
        _swapChain = &swapChain;
        _resourceManager = &resourceManager;
		_graphicsCommandPool = &graphicsCommandPool;

        _cameraMatrixPushConstantHandle = ResourceHandle<UniformBufferResource>(cameraMatrixResourceId, _resourceManager);

		_renderGraph = new hl::GeneratedRenderGraph(
			device,
			swapChain,
			renderpassInfo,
			resourceManager);

		size_t maxPipelineGroups = 0;
		for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); layer++)
		{
			for (const auto& renderpassName : _renderGraph->getSortedNodesByNameForLayer(layer))
			{
				const auto& renderpass = _renderGraph->getRenderpassByName(renderpassName);
				maxPipelineGroups = std::max(maxPipelineGroups, renderpass->getPipelineGroups().size());
			}
		}

		if (maxPipelineGroups > 1)
		{
			_secondaryRecordPools.reserve(maxPipelineGroups - 1);
			for (size_t i = 1; i < maxPipelineGroups; ++i)
			{
				auto pool = std::make_unique<VulkanCommandPool>(device);
				pool->create();
				_secondaryRecordPools.push_back(std::move(pool));
			}
		}

		_frameResources.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo primaryAllocInfo{};
		primaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		primaryAllocInfo.commandPool = graphicsCommandPool.handle();
		primaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		primaryAllocInfo.commandBufferCount = (uint32_t)_frameResources.size();

		std::vector<VkCommandBuffer> perFrameCommandBuffers(MAX_FRAMES_IN_FLIGHT);

		CHECK_VK_RESULT(vkAllocateCommandBuffers(device.handle(), &primaryAllocInfo, perFrameCommandBuffers.data()));

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

					if (renderpass->isCompute())
					{
						secondaryCommandsForGroups.clear();
						continue;
					}

					secondaryCommandsForGroups.resize(renderpass->getPipelineGroups().size());

					uint32_t groupIndex = 0;
					for (auto& secondaryCommand : secondaryCommandsForGroups)
					{
						VkCommandPool commandPool = groupIndex == 0
							? graphicsCommandPool.handle()
							: _secondaryRecordPools[groupIndex - 1]->handle();

						VkCommandBufferAllocateInfo secondaryAllocInfo{};
						secondaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
						secondaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
						secondaryAllocInfo.commandPool = commandPool;
						secondaryAllocInfo.commandBufferCount = 1;

						CHECK_VK_RESULT(vkAllocateCommandBuffers(device.handle(), &secondaryAllocInfo, &secondaryCommand));

						device.setDebugName(
							reinterpret_cast<uint64_t>(secondaryCommand),
							VK_OBJECT_TYPE_COMMAND_BUFFER,
							("SecondaryCommandBuffer_" + std::to_string(i) + "_" + renderpass->Name + "_Group_" + std::to_string(groupIndex)).c_str());

						groupIndex++;
					}
				}
			}
		}

		for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); ++layer)
		{
			for (const auto& renderpassName : _renderGraph->getSortedNodesByNameForLayer(layer))
			{
				const auto& renderpass = _renderGraph->getRenderpassByName(renderpassName);
				if (!renderpass->isCompute())
				{
					continue;
				}

				for (const auto& group : renderpass->getPipelineGroups())
				{
					for (auto* pipeline : group)
					{
						if (_pipelineDraws.contains(pipeline->Name))
						{
							continue;
						}

						if (pipeline->Name == ParticleSystem::ComputePipelineName)
						{
							registerPipelineDraw(pipeline->Name, [this](PipelineDrawData& pdd)
								{
									_engine.getParticleSystem().recordCompute(
										pdd.commandBuffer,
										pdd.pipeline,
										pdd.currentFrame);
								});
							continue;
						}

						registerPipelineDraw(pipeline->Name, [](PipelineDrawData& pdd)
							{
								ZoneScopedN("compute dispatch");
								vkCmdBindPipeline(
									pdd.commandBuffer,
									VK_PIPELINE_BIND_POINT_COMPUTE,
									pdd.pipeline->getPipeline());
								vkCmdDispatch(pdd.commandBuffer, 1, 1, 1);
							});
					}
				}
			}
		}

		for (uint32_t layer = 0; layer < _renderGraph->getNumberLayers(); ++layer)
		{
			for (const auto& renderpassName : _renderGraph->getSortedNodesByNameForLayer(layer))
			{
				const auto& renderpass = _renderGraph->getRenderpassByName(renderpassName);
				for (const auto& group : renderpass->getPipelineGroups())
				{
					for (auto* pipeline : group)
					{
						if (_pipelineDraws.contains(pipeline->Name))
						{
							continue;
						}

						if (pipeline->Name == ParticleSystem::DrawPipelineName)
						{
							registerPipelineDraw(pipeline->Name, [this](PipelineDrawData& pdd)
								{
									_engine.getParticleSystem().recordDraw(
										pdd.commandBuffer,
										pdd.pipeline,
										pdd.currentFrame);
								});
						}
						else if (pipeline->Name == ParticleSystem::QuadPipelineName)
						{
							registerPipelineDraw(pipeline->Name, [this](PipelineDrawData& pdd)
								{
									_engine.getParticleSystem().recordDrawQuads(
										pdd.commandBuffer,
										pdd.pipeline,
										pdd.currentFrame);
								});
						}
					}
				}
			}
		}

        for (auto& [name, camera] : _cameras)
        {
            camera->notifyFramebufferChangeSize((uint32_t)swapChain.extent().width, (uint32_t)swapChain.extent().height);
        }        
	}

	void EngineScene::initialise(
        const std::string& cameraMatrixResourceId,
		VulkanDevice& device,
		VulkanSwapChain& swapChain,
        VulkanCommandPool& graphicsCommandPool,
        VulkanCommandPool& transferCommandPool,
		ResourceManager& resourceManager)
	{
		initialise(cameraMatrixResourceId, device, swapChain, graphicsCommandPool, transferCommandPool, resourceManager, {});
	}
	void EngineScene::cleanup()
	{
        additionalCleanup();
		_renderGraph->destroy();
		delete _renderGraph;
		_renderGraph = nullptr;

		for (auto& pool : _secondaryRecordPools)
		{
			pool->destroy();
		}
		_secondaryRecordPools.clear();
		_graphicsCommandPool = nullptr;
	}

    void EngineScene::updateBase(uint32_t currentFrame, float delta)
    {
        ZoneScopedN("Engine Scene Update");
        update(currentFrame, delta);
        _engine.getInputManager().updateEndOfFrame();
        _scene.update();
    }
	void EngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
	}
    void EngineScene::updateGpuResources(uint32_t /*currentFrame*/)
    {

    }
	VkCommandBuffer EngineScene::draw(uint32_t currentFrame, uint32_t imageIndex)
	{
		ZoneScopedN("Engine Scene Draw");

		assert(_device != nullptr);
		assert(_swapChain != nullptr);
		assert(_renderGraph != nullptr);
		assert(_device->cmdBeginRendering() != nullptr);
		assert(_device->cmdEndRendering() != nullptr);
		assert(currentFrame < static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
		assert(imageIndex < _swapChain->images().size());
		assert(imageIndex < _swapChain->imageViews().size());
		assert(!_renderGraph->getResources().empty());

		updateCameraUniformBuffer(_cameraMatrixPushConstantHandle.Get()->getUniformBuffer(currentFrame));

        const auto& lastRenderpassName = _renderGraph->getResources().back()->Name;
        auto& frame = _frameResources[currentFrame];
		assert(frame.primaryCmd != VK_NULL_HANDLE);

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

            size_t renderpassIndex = 0;
            for (const auto& renderpassName : _renderGraph->getSortedNodesByNameForLayer(layer))
            {
                ZoneScoped;
                ZoneNameF("record command buffer for %s", renderpassName.c_str());

				const auto& renderpass = _renderGraph->getRenderpassByName(renderpassName);
				assert(renderpass != nullptr);

				if (renderpass->isCompute())
				{
					_renderGraph->recordPrePassBarriers(
						frame.primaryCmd,
						renderpassName,
						currentFrame,
						imageIndex);

					for (const auto& group : renderpass->getPipelineGroups())
					{
						for (auto* pipeline : group)
						{
							renderPipelineDraw(
								frame.primaryCmd,
								renderpassName,
								pipeline,
								currentFrame);
						}
					}

					_renderGraph->recordPostPassBarriers(
						frame.primaryCmd,
						renderpassName,
						imageIndex);

					++renderpassIndex;
					continue;
				}

                const auto& clearValues = renderpass->getClearValues();
                const bool isLastRenderpass = lastRenderpassName == renderpass->Name;
                const uint32_t attachmentSlot = isLastRenderpass ? imageIndex : currentFrame;
                const bool msaa = renderpass->usesMultiSampling();
                const auto renderExtent = getExtent(renderpass->getExtent(), _swapChain->extent());
				assert(renderExtent.width > 0 && renderExtent.height > 0);

                _renderGraph->recordPrePassBarriers(
                    frame.primaryCmd,
                    renderpassName,
                    currentFrame,
                    imageIndex);

                std::vector<VkRenderingAttachmentInfo> colorAttachments;
                VkRenderingAttachmentInfo depthAttachment{};
                bool hasDepth = false;
                size_t clearIndex = 0;

                for (const auto& attachment : renderpass->getAttachments())
                {
                    if (attachment.type == ResourceType::Color)
                    {
                        VkRenderingAttachmentInfo colorInfo{};
                        colorInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                        colorInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

                        assert(clearIndex < clearValues.size());
                        colorInfo.clearValue = clearValues[clearIndex++];

                        if (isLastRenderpass && !msaa)
                        {
                            colorInfo.imageView = _swapChain->imageViews()[imageIndex];
                        }
                        else
                        {
                            assert(attachmentSlot < attachment.images.size());
                            assert(attachment.images[attachmentSlot] != nullptr);
                            colorInfo.imageView = attachment.images[attachmentSlot]->_imageView;
                        }
						assert(colorInfo.imageView != VK_NULL_HANDLE);

                        if (msaa)
                        {
                            if (clearIndex < clearValues.size())
                            {
                                ++clearIndex;
                            }
                            colorInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                            colorInfo.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            if (isLastRenderpass)
                            {
                                colorInfo.resolveImageView = _swapChain->imageViews()[imageIndex];
                            }
                            else
                            {
                                assert(attachmentSlot < attachment.resolveImages.size());
                                assert(attachment.resolveImages[attachmentSlot] != nullptr);
                                colorInfo.resolveImageView = attachment.resolveImages[attachmentSlot]->_imageView;
                            }
							assert(colorInfo.resolveImageView != VK_NULL_HANDLE);
                        }

                        colorAttachments.push_back(colorInfo);
                    }
                    else if (attachment.type == ResourceType::Depth)
                    {
                        hasDepth = true;
                        assert(attachmentSlot < attachment.images.size());
                        assert(attachment.images[attachmentSlot] != nullptr);
                        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                        depthAttachment.imageView = attachment.images[attachmentSlot]->_imageView;
						assert(depthAttachment.imageView != VK_NULL_HANDLE);
                        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        assert(clearIndex < clearValues.size());
                        depthAttachment.clearValue = clearValues[clearIndex++];
                    }
                }

                VkRenderingInfo renderingInfo{};
                renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
                renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
                renderingInfo.renderArea = { .offset = { 0, 0 }, .extent = renderExtent };
                renderingInfo.layerCount = 1;
                renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
                renderingInfo.pColorAttachments = colorAttachments.empty() ? nullptr : colorAttachments.data();
                renderingInfo.pDepthAttachment = hasDepth ? &depthAttachment : nullptr;
				assert(renderingInfo.layerCount == 1);
				assert(renderingInfo.colorAttachmentCount == renderpass->getColorFormats().size());

                _device->cmdBeginRendering()(frame.primaryCmd, &renderingInfo);

                VkCommandBufferInheritanceRenderingInfo inheritanceRendering{};
                inheritanceRendering.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                inheritanceRendering.colorAttachmentCount = static_cast<uint32_t>(renderpass->getColorFormats().size());
                inheritanceRendering.pColorAttachmentFormats = renderpass->getColorFormats().empty()
                    ? nullptr
                    : renderpass->getColorFormats().data();
                inheritanceRendering.depthAttachmentFormat = renderpass->getDepthFormat();
                inheritanceRendering.rasterizationSamples = renderpass->getRasterizationSamples();

                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.pNext = &inheritanceRendering;
                inheritanceInfo.renderPass = VK_NULL_HANDLE;
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;

                VkCommandBufferBeginInfo secondaryBeginInfo{};
                secondaryBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                secondaryBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                secondaryBeginInfo.pInheritanceInfo = &inheritanceInfo;

                const auto& secondaryBuffersForGroup = secondaryBuffersPerRenderpass[renderpassIndex];
                const auto& pipelineGroups = renderpass->getPipelineGroups();

                if (pipelineGroups.size() > 1)
                {
                    std::vector<std::future<void>> futures;
                    futures.reserve(pipelineGroups.size() - 1);
                    for (size_t groupIndex = 1; groupIndex < pipelineGroups.size(); ++groupIndex)
                    {
                        futures.push_back(std::async(
                            std::launch::async,
                            [this, &secondaryBeginInfo, renderpass, &secondaryBuffersForGroup, &renderpassName, currentFrame, groupIndex]()
                            {
                                recordPipelineGroup(
                                    secondaryBuffersForGroup[groupIndex],
                                    secondaryBeginInfo,
                                    *renderpass,
                                    groupIndex,
                                    renderpassName,
                                    currentFrame);
                            }));
                    }

                    recordPipelineGroup(
                        secondaryBuffersForGroup[0],
                        secondaryBeginInfo,
                        *renderpass,
                        0,
                        renderpassName,
                        currentFrame);

                    for (auto& future : futures)
                    {
                        future.get();
                    }
                }
                else if (pipelineGroups.size() == 1)
                {
                    recordPipelineGroup(
                        secondaryBuffersForGroup[0],
                        secondaryBeginInfo,
                        *renderpass,
                        0,
                        renderpassName,
                        currentFrame);
                }

                if (!secondaryBuffersForGroup.empty())
                {
                    vkCmdExecuteCommands(
                        frame.primaryCmd,
                        static_cast<uint32_t>(secondaryBuffersForGroup.size()),
                        secondaryBuffersForGroup.data());
                }

                _device->cmdEndRendering()(frame.primaryCmd);

                _renderGraph->recordPostPassBarriers(
                    frame.primaryCmd,
                    renderpassName,
                    imageIndex);

                ++renderpassIndex;
            }
        }

        CHECK_VK_RESULT(vkEndCommandBuffer(frame.primaryCmd));

        return frame.primaryCmd;
	}

	void EngineScene::recreate(uint32_t width, uint32_t height)
	{
		_renderGraph->recreate(width, height);

        for (auto& [name, camera] : _cameras)
        {
            camera->notifyFramebufferChangeSize(width, height);
        }

		syncCameraUniformBuffers();
	}
	void EngineScene::syncCameraUniformBuffers()
	{
		auto* cameras = _cameraMatrixPushConstantHandle.Get();
		if (cameras == nullptr)
		{
			return;
		}

		for (uint32_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame)
		{
			updateCameraUniformBuffer(cameras->getUniformBuffer(frame));
		}
	}
	void EngineScene::updateAllDescriptorSets()
	{
		_renderGraph->updateAllDescriptorSets();
	}
    void EngineScene::updateAllOutputResources()
    {
        _renderGraph->updateAllOutputResources();
    }
    void EngineScene::registerPipelineDraw(const std::string& pipelineName, std::function<void(PipelineDrawData&)> pipelineDraw)
    {
        _pipelineDraws.insert({ pipelineName, pipelineDraw });
    }
    std::size_t EngineScene::getCameraIndex(const std::string& cameraName) const
    {
        std::size_t idx = 0;
        for (auto& [name, camera] : _cameras)
        {
            if (name == cameraName)
            {
                return idx;
            }

            idx++;
        }

        // TODO: ERROR?
        return 0;
    }
    void EngineScene::updateCameraUniformBuffer(VulkanUniformBuffer& uniformBuffer)
    {
        std::size_t idx = 0;
        for (auto& [name, camera] : _cameras)
        {
            CameraUniformBufferObject ubo{};

            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();

            ubo.proj[1][1] *= -1;

            uniformBuffer.writeToBuffer(&ubo, idx);

            idx++;
        }
    }
    void EngineScene::recordPipelineGroup(
        VkCommandBuffer secondaryBuffer,
        const VkCommandBufferBeginInfo& secondaryBeginInfo,
        VulkanRenderGraphRenderpassResources& renderpass,
        size_t pipelineGroupIndex,
        const std::string& renderpassName,
        uint32_t currentFrame)
    {
        CHECK_VK_RESULT(vkBeginCommandBuffer(secondaryBuffer, &secondaryBeginInfo));

        const auto& pg = renderpass.getPipelineGroups()[pipelineGroupIndex];
        for (const auto& p : pg)
        {
            vkCmdBindPipeline(secondaryBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p->getPipeline());

            const auto viewportConfigWidth = p->getViewportWidth();
            const auto viewportConfigHeight = p->getViewportHeight();

            float viewportX = 0.0f;
            float viewportY = 0.0f;
            float viewportWidth = 0.0f;
            float viewportHeight = 0.0f;

            switch (p->getViewportMode())
            {
            case ViewportMode::FixedAspect:
                {
                    const auto extent =
                        getExtent(renderpass.getExtent(), _swapChain->extent());

                    const float windowWidth = static_cast<float>(extent.width);
                    const float windowHeight = static_cast<float>(extent.height);

                    const auto windowAspectRatio =
                        windowWidth / windowHeight;

                    const auto viewportAspectRatio =
                        static_cast<float>(viewportConfigWidth) /
                        static_cast<float>(viewportConfigHeight);

                    if (windowAspectRatio > viewportAspectRatio)
                    {
                        viewportHeight = windowHeight;
                        viewportWidth = viewportHeight * viewportAspectRatio;

                        const float unusedWidth = windowWidth - viewportWidth;

                        viewportX = unusedWidth / 2.0f;
                        viewportY = 0.0f;
                    }
                    else
                    {
                        viewportWidth = windowWidth;
                        viewportHeight = viewportWidth / viewportAspectRatio;

                        const float unusedHeight = windowHeight - viewportHeight;

                        viewportX = 0.0f;
                        viewportY = unusedHeight / 2.0f;
                    }
                }
                break;
            case ViewportMode::FixedResolution:
                throw std::runtime_error("TODO");
                break;
            case ViewportMode::Custom:
                throw std::runtime_error("TODO");
                break;
            case ViewportMode::Fill:
            default:
                viewportX = 0;
                viewportY = 0;
                viewportWidth = (float)getExtent(renderpass.getExtent(), _swapChain->extent()).width;
                viewportHeight = (float)getExtent(renderpass.getExtent(), _swapChain->extent()).height;
                break;
            }

            {
                VkViewport viewport
                {
                    .x = viewportX,
                    .y = viewportY,
                    .width = viewportWidth,
                    .height = viewportHeight,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                };
                vkCmdSetViewport(secondaryBuffer, 0, 1, &viewport);

                VkRect2D scissor
                {
                    .offset =
                    {
                        static_cast<int32_t>(viewportX),
                        static_cast<int32_t>(viewportY)
                    },
                    .extent =
                    {
                        static_cast<uint32_t>(viewportWidth),
                        static_cast<uint32_t>(viewportHeight)
                    }
                };
                vkCmdSetScissor(secondaryBuffer, 0, 1, &scissor);
            }

            renderPipelineDraw(secondaryBuffer, renderpassName, p, currentFrame);
        }

        CHECK_VK_RESULT(vkEndCommandBuffer(secondaryBuffer));
    }

    void EngineScene::renderPipelineDraw(
        VkCommandBuffer commandBuffer,
        const std::string& renderpassName, 
        hl::VulkanRenderGraphPipelineResources* pipeline, 
        uint32_t currentFrame)
    {
        auto iter = _pipelineDraws.find(pipeline->Name);

        PipelineDrawData pdd
        {
            .currentFrame = currentFrame,
            .commandBuffer = commandBuffer,
            .pipeline = pipeline,
            .scene = &_scene
        };

        if (iter != _pipelineDraws.end())
        {
            (*iter).second(pdd);
        }
        else if (pipeline->Name == "skybox_pipeline")
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
        else if (pipeline->Name == "model_pipeline"
            || pipeline->Name == RenderGraphHelpers::ShadowPipelineName)
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
                    pc.materialIndex = _engine.getMaterialSystem().getMaterialIndex(mesh.materialName);
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
        else if (pipeline->Name == "text_pipeline" ||
                 pipeline->Name == "sdf_text_pipeline")
        {
            auto& textSystem = _engine.getTextSystem();

            auto fontType = pipeline->Name == "text_pipeline"
                ? hl::FontType::Rasterised
                : hl::FontType::SignedDistanceField;

            for (const auto& entity : _scene.getEntities())
            {
                if (!entity->HasComponents<hl::TransformComponent, hl::TextComponent>())
                {
                    continue;
                }

                const auto& transform = entity->GetComponent<hl::TransformComponent>();
                const auto& text = entity->GetComponent<hl::TextComponent>();

                const auto& t = textSystem.getText(text->getTextSystemId());

                if (t._fontType != fontType)
                {
                    continue;
                }

                auto modelTransform = transform->GetTransformMatrix();

                auto pc = hl::TextPushConstantObject
                {
                    .model = modelTransform,
                    .colour = text->getColour(),
                    .fontAtlasIndex = textSystem.getFontAtlasIndex(fontType, text->getFont())
                };

                vkCmdPushConstants(
                    commandBuffer,
                    pipeline->getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(hl::TextPushConstantObject),
                    &pc
                );

                VkBuffer vertexBuffers[] = { t._vertexBuffer._buffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(
                    commandBuffer,
                    0,
                    1,
                    vertexBuffers,
                    offsets);

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

                vkCmdDraw(commandBuffer, t._vertexCount, 1, 0, 0);
            }
        }
        else
        {
            std::cout << "Renderpass: '" << renderpassName << "' has unhandled pipeline draw: '" << pipeline->Name << "'" << std::endl;
        }
    }
}