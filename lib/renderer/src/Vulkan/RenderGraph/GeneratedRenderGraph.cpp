#include <helsinki/Renderer/Vulkan/RenderGraph/GeneratedRenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Resource/OffscreenImageResource.hpp>
#include <helsinki/System/Resource/LogicalResource.hpp>
#include <cassert>
#include <stdexcept>

namespace hl
{

	GeneratedRenderGraph::GeneratedRenderGraph(
        VulkanDevice& device,
        VulkanSwapChain& swapChain,
		std::vector<hl::RenderpassInfo> renderpasses,
        ResourceManager& resourceManager
	) :
        _device(device),
        _swapChain(swapChain),
		_renderGraph(renderpasses),
        _dag(hl::RenderGraph::generateDAG(renderpasses)),
        _resourceManager(resourceManager)
	{
		_resources = hl::RenderGraph::create(
			renderpasses,
			device,
			swapChain.extent().width,
			swapChain.extent().height,
			swapChain.imageViews(),
			swapChain.format(),
            resourceManager);

        for (uint32_t layer = 0;; ++layer)
        {
            bool found = false;
            for (auto& [_, node] : _dag)
            {
                if (node.layer == layer)
                {
                    _sortedNodes.push_back(&node);
                    found = true;
                }
            }

            if (!found) 
            { 
                break; 
            }
        }

        for (auto& [name, node] : _dag)
        {
            _nodesByLayer[node.layer].push_back(name);
        }
	}

    VkDescriptorSet GeneratedRenderGraph::getDescriptorSet(const std::string& renderpassName, const std::string& pipelineName, uint32_t frameNumber)
    {
        for (auto& r : _resources)
        {
            if (r->Name == renderpassName)
            {
                for (auto& pg : r->getPipelineGroups())
                {
                    for (auto& p : pg)
                    {
                        if (p->Name == pipelineName)
                        {
                            return p->getDescriptorSet(frameNumber);
                        }
                    }
                }
            }
        }

        throw std::runtime_error("failed to find descriptor set");
    }

    std::vector<VulkanRenderGraphRenderpassResources*> GeneratedRenderGraph::getResources()
	{
		return _resources;
	}
    VulkanRenderGraphRenderpassResources* GeneratedRenderGraph::getRenderpassByName(const std::string& name)
    {
        for (auto& r : _resources)
        {
            if (r->Name == name)
            {
                return r;
            }
        }

        throw std::runtime_error("Could not find renderpass with given name");
    }
    const std::vector<hl::RenderpassInfo>& GeneratedRenderGraph::getRenderpassInfo() const
    {
        return _renderGraph;
    }

    std::vector<std::string> GeneratedRenderGraph::getSortedNodesByName() const
    {
        std::vector<std::string> names;

        for (const auto& s : _sortedNodes)
        {
            names.push_back(s->name);
        }

        return names;
    }

    const std::vector<std::string>& GeneratedRenderGraph::getSortedNodesByNameForLayer(uint32_t layer) const
    {
        return _nodesByLayer.at(layer);
    }

    uint32_t GeneratedRenderGraph::getNumberLayers() const
    {
        return (uint32_t)_nodesByLayer.size();
    }

	void GeneratedRenderGraph::destroy()
	{
		hl::RenderGraph::destroy(_resources);
	}

	void GeneratedRenderGraph::recreate(uint32_t width, uint32_t height)
	{
        size_t i = 0;
        for (auto& r : getResources())
        {
            const auto& info = _renderGraph[i];
            i++;

            auto isLastRenderpass = i == getResources().size();

            r->recreate(
                info,
                width,
                height,
                (uint32_t)(isLastRenderpass ? _swapChain.imageViews().size() : MAX_FRAMES_IN_FLIGHT),
                isLastRenderpass);

            updateAllOutputResources();
        }
	}

	void GeneratedRenderGraph::updateAllDescriptorSets()
	{
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (auto& r : _renderGraph)
            {
                for (auto& pg : r.pipelineGroups)
                {
                    for (auto& p : pg)
                    {
                        std::vector<VkWriteDescriptorSet> descriptorWrites;

                        size_t imageInfoCount = 0;
                        size_t bufferInfoCount = 0;

                        for (auto& ds : p.descriptorSets)
                        {
                            for (auto& b : ds.bindings)
                            {
                                if (!shouldWriteDescriptorBinding(b.updateFrequency, _staticDescriptorsWritten))
                                {
                                    continue;
                                }

                                if (b.type == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER")
                                {
                                    bufferInfoCount += 1;
                                }
                                else if (b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER" ||
                                    b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC")
                                {
                                    bufferInfoCount += b.count;
                                }
                                else if (b.type == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER")
                                {
                                    imageInfoCount += b.count;
                                }
                            }
                        }

                        std::vector<VkDescriptorImageInfo> imageInfos;
                        std::vector<VkDescriptorBufferInfo> bufferInfos;

                        imageInfos.reserve(imageInfoCount);
                        bufferInfos.reserve(bufferInfoCount);

                        for (auto& ds : p.descriptorSets)
                        {
                            for (auto& b : ds.bindings)
                            {
                                if (!shouldWriteDescriptorBinding(b.updateFrequency, _staticDescriptorsWritten))
                                {
                                    continue;
                                }

                                if (b.resource.has_value())
                                {
                                    if (b.type == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER")
                                    {
                                        auto& ub = _resourceManager
                                            .GetResource<StorageBufferResource>(
                                                b.resource.value())
                                            ->getBuffer();

                                        bufferInfos.push_back(VkDescriptorBufferInfo
                                            {
                                                .buffer = ub._buffer,
                                                .offset = 0,
                                                .range = VK_WHOLE_SIZE
                                            });

                                        descriptorWrites.emplace_back(VkWriteDescriptorSet
                                            {
                                                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                .dstSet = getDescriptorSet(r.name, p.name, i),
                                                .dstBinding = b.binding,
                                                .dstArrayElement = 0,
                                                .descriptorCount = 1,
                                                .descriptorType = RenderGraph::extractDescriptorType(b.type),
                                                .pBufferInfo = &bufferInfos.back()
                                            });
                                    }
                                    else if (b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER" ||
                                        b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC")
                                    {
                                        auto& ub = _resourceManager
                                            .GetResource<UniformBufferResource>(
                                                b.resource.value())
                                            ->getUniformBuffer(i);

                                        auto bufferInfoStart = bufferInfos.size();

                                        for (uint32_t descriptorBufferIndex = 0; descriptorBufferIndex < b.count; ++descriptorBufferIndex)
                                        {
                                            bufferInfos.push_back(
                                                VkDescriptorBufferInfo{
                                                    .buffer = ub._buffer._buffer,
                                                    .offset = descriptorBufferIndex * ub._size,
                                                    .range = ub._size
                                                });
                                        }

                                        descriptorWrites.emplace_back(VkWriteDescriptorSet
                                            {
                                                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                .dstSet = getDescriptorSet(r.name, p.name, i),
                                                .dstBinding = b.binding,
                                                .dstArrayElement = 0,
                                                .descriptorCount = b.count,
                                                .descriptorType = RenderGraph::extractDescriptorType(b.type),   
                                                .pBufferInfo = &bufferInfos[bufferInfoStart]
                                            });
                                    }
                                    else if (b.type == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER")
                                    {
                                        if (b.count > 1)
                                        {
                                            auto* logical = _resourceManager
                                                .GetResource<LogicalResource>(b.resource.value());

                                            if (logical == nullptr)
                                            {
                                                throw std::runtime_error("Cannot handle hard coded image sampler arrays");
                                            }

                                            const auto imageInfoStart = imageInfos.size();

                                            for (const auto& childName : logical->GetChildren())
                                            {
                                                const auto info = _resourceManager
                                                    .GetResource<ImageSamplerResource>(childName)
                                                    ->getDescriptorInfo(i);

                                                imageInfos.push_back(VkDescriptorImageInfo
                                                    {
                                                        .sampler = info.first,
                                                        .imageView = info.second,
                                                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                    });
                                            }

                                            const auto childCount = static_cast<uint32_t>(imageInfos.size() - imageInfoStart);
                                            if (childCount == 0)
                                            {
                                                throw std::runtime_error("Logical resource has no children for sampler array binding");
                                            }
                                            if (childCount > b.count)
                                            {
                                                throw std::runtime_error("Logical resource has more children than sampler array binding count");
                                            }

                                            const auto writeCount = hl::descriptorArrayWriteCount(b.count, childCount, b.partiallyBound);
                                            if (!b.partiallyBound)
                                            {
                                                const auto& fallbackInfo = imageInfos[imageInfoStart];
                                                while (imageInfos.size() - imageInfoStart < writeCount)
                                                {
                                                    imageInfos.push_back(fallbackInfo);
                                                }
                                            }

                                            descriptorWrites.emplace_back(VkWriteDescriptorSet
                                                {
                                                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                    .dstSet = getDescriptorSet(r.name, p.name, i),
                                                    .dstBinding = b.binding,
                                                    .dstArrayElement = 0,
                                                    .descriptorCount = writeCount,
                                                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                    .pImageInfo = &imageInfos[imageInfoStart]
                                                });
                                        }
                                        else
                                        {
                                            const auto info = _resourceManager
                                                .GetResource<ImageSamplerResource>(
                                                    b.resource.value())
                                                ->getDescriptorInfo(i);

                                            imageInfos.push_back(VkDescriptorImageInfo
                                                {
                                                    .sampler = info.first,
                                                    .imageView = info.second,
                                                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                });

                                            descriptorWrites.emplace_back(VkWriteDescriptorSet
                                                {
                                                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                    .dstSet = getDescriptorSet(r.name, p.name, i),
                                                    .dstBinding = b.binding,
                                                    .dstArrayElement = 0,
                                                    .descriptorCount = 1,
                                                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                    .pImageInfo = &imageInfos.back()
                                                });
                                        }
                                    }
                                    else
                                    {
                                        throw std::runtime_error("TODO: unhandled descriptor binding type");
                                    }
                                }
                            }
                        }

                        if (!descriptorWrites.empty())
                        {
                            vkUpdateDescriptorSets(
                                _device.handle(),
                                static_cast<uint32_t>(descriptorWrites.size()),
                                descriptorWrites.data(),
                                0,
                                nullptr);
                        }
                    }
                }
            }
        }

        _staticDescriptorsWritten = true;
	}

    void GeneratedRenderGraph::updateAllOutputResources()
    {
        for (const auto& r : _renderGraph)
        {
            for (const auto& pg : r.pipelineGroups)
            {
                for (const auto& p : pg)
                {
                    for (const auto& ds : p.descriptorSets)
                    {
                        for (const auto& b : ds.bindings)
                        {
                            if (b.resource.has_value())
                            {
                                for (const auto& grpr : _resources)
                                {
                                    if (grpr->Name != r.name)
                                    {
                                        for (auto& grpra : grpr->getAttachments())
                                        {
                                            if (grpra.name == b.resource.value())
                                            {
                                                std::vector<hl::VulkanImage*> offscreenImages;

                                                if (!grpra.resolveImages.empty())
                                                {
                                                    for (auto i : grpra.resolveImages)
                                                    {
                                                        offscreenImages.push_back(i);
                                                    }
                                                }
                                                else
                                                {
                                                    for (auto i : grpra.images)
                                                    {
                                                        offscreenImages.push_back(i);
                                                    }
                                                }

                                                if (grpra.sampler == VK_NULL_HANDLE)
                                                {
                                                    // TODO: config
                                                    VkSamplerCreateInfo samplerInfo{};
                                                    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                                                    samplerInfo.magFilter = VK_FILTER_LINEAR;
                                                    samplerInfo.minFilter = VK_FILTER_LINEAR;
                                                    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                                                    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                                                    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                                                    samplerInfo.anisotropyEnable = VK_FALSE;
                                                    samplerInfo.maxAnisotropy = 1.0f;
                                                    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                                                    samplerInfo.unnormalizedCoordinates = VK_FALSE;
                                                    samplerInfo.compareEnable = VK_FALSE;
                                                    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                                                    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                                                    samplerInfo.mipLodBias = 0.0f;
                                                    samplerInfo.minLod = 0.0f;
                                                    samplerInfo.maxLod = 0.0f;

                                                    CHECK_VK_RESULT(vkCreateSampler(_device.handle(), &samplerInfo, nullptr, &grpra.sampler));

                                                    _device.setDebugName(
                                                        reinterpret_cast<uint64_t>(grpra.sampler),
                                                        VK_OBJECT_TYPE_SAMPLER,
                                                        (r.name + "_" + grpra.name + "_Sampler").c_str());
                                                }

                                                // Create load if does not exist?
                                                if (!_resourceManager.HasResource<ImageSamplerResource>(b.resource.value()))
                                                {
                                                    _resourceManager.LoadAs<OffscreenImageResource, ImageSamplerResource>(b.resource.value());
                                                }

                                                auto samplerResource = _resourceManager.GetResourceAs<OffscreenImageResource, ImageSamplerResource>(b.resource.value());

                                                std::vector<VkImageView> imageViews;

                                                for (auto& image : offscreenImages)
                                                {
                                                    imageViews.push_back(image->_imageView);
                                                }

                                                samplerResource->set(grpra.sampler, imageViews);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

	static const RenderpassAttachment* findAttachmentByName(
		const std::vector<VulkanRenderGraphRenderpassResources*>& resources,
		const std::string& resourceName)
	{
		for (const auto* pass : resources)
		{
			for (const auto& attachment : pass->getAttachments())
			{
				if (attachment.name == resourceName)
				{
					return &attachment;
				}
			}
		}

		return nullptr;
	}

	void GeneratedRenderGraph::recordPrePassBarriers(
		VkCommandBuffer commandBuffer,
		const std::string& passName,
		uint32_t currentFrame,
		uint32_t imageIndex)
	{
		assert(commandBuffer != VK_NULL_HANDLE);
		assert(imageIndex < _swapChain.images().size());
		assert(currentFrame < static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

		auto* pass = getRenderpassByName(passName);
		assert(pass != nullptr);
		const uint32_t slot = pass->writesToSwapchain() ? imageIndex : currentFrame;
		const bool msaa = pass->usesMultiSampling();

		for (const auto& edge : RenderGraph::generateImageBarrierEdges(_renderGraph))
		{
			if (edge.passName != passName)
			{
				continue;
			}

			if (edge.kind == GraphImageBarrierKind::UndefinedToColorAttachment)
			{
				const auto* attachment = findAttachmentByName(_resources, edge.resourceName);
				assert(attachment != nullptr);

				if (pass->writesToSwapchain() && !msaa)
				{
					VulkanCommandBuffer::undefinedToColorAttachment(_device, commandBuffer, _swapChain.images()[imageIndex]);
				}
				else
				{
					assert(slot < attachment->images.size());
					assert(attachment->images[slot] != nullptr);
					VulkanCommandBuffer::undefinedToColorAttachment(_device, commandBuffer, attachment->images[slot]->_image);
					if (msaa)
					{
						if (pass->writesToSwapchain())
						{
							VulkanCommandBuffer::undefinedToColorAttachment(_device, commandBuffer, _swapChain.images()[imageIndex]);
						}
						else
						{
							assert(slot < attachment->resolveImages.size());
							assert(attachment->resolveImages[slot] != nullptr);
							VulkanCommandBuffer::undefinedToColorAttachment(_device, commandBuffer, attachment->resolveImages[slot]->_image);
						}
					}
				}
			}
			else if (edge.kind == GraphImageBarrierKind::UndefinedToDepthAttachment)
			{
				const auto* attachment = findAttachmentByName(_resources, edge.resourceName);
				assert(attachment != nullptr);
				assert(slot < attachment->images.size());
				assert(attachment->images[slot] != nullptr);
				VulkanCommandBuffer::undefinedToDepthAttachment(_device, commandBuffer, attachment->images[slot]->_image);
			}
			else if (edge.kind == GraphImageBarrierKind::ColorAttachmentToSampled)
			{
				const auto* attachment = findAttachmentByName(_resources, edge.resourceName);
				assert(attachment != nullptr);

				const uint32_t sampledSlot = currentFrame;
				if (!attachment->resolveImages.empty())
				{
					assert(sampledSlot < attachment->resolveImages.size());
					assert(attachment->resolveImages[sampledSlot] != nullptr);
					VulkanCommandBuffer::colorAttachmentToSampled(_device, commandBuffer, attachment->resolveImages[sampledSlot]->_image);
				}
				else
				{
					assert(sampledSlot < attachment->images.size());
					assert(attachment->images[sampledSlot] != nullptr);
					VulkanCommandBuffer::colorAttachmentToSampled(_device, commandBuffer, attachment->images[sampledSlot]->_image);
				}
			}
		}

		for (const auto& edge : RenderGraph::generateBufferBarrierEdges(_renderGraph))
		{
			if (edge.passName != passName)
			{
				continue;
			}

			auto& buffer = _resourceManager
				.GetResource<StorageBufferResource>(edge.resourceName)
				->getBuffer();
			VulkanCommandBuffer::storageWriteToVertexRead(_device, commandBuffer, buffer._buffer);
		}
	}

	void GeneratedRenderGraph::recordPostPassBarriers(
		VkCommandBuffer commandBuffer,
		const std::string& passName,
		uint32_t imageIndex)
	{
		assert(commandBuffer != VK_NULL_HANDLE);
		assert(imageIndex < _swapChain.images().size());

		for (const auto& edge : RenderGraph::generateImageBarrierEdges(_renderGraph))
		{
			if (edge.passName != passName || edge.kind != GraphImageBarrierKind::ColorAttachmentToPresent)
			{
				continue;
			}

			VulkanCommandBuffer::colorAttachmentToPresent(_device, commandBuffer, _swapChain.images()[imageIndex]);
		}
	}
}