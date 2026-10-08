#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/HelsinkiTracy.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace hl
{

	static std::string readFileContents(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		if (!file)
		{
			throw std::runtime_error("Failed to open file: " + filename);
		}

		std::ostringstream contents;
		contents << file.rdbuf();
		return contents.str();
	}

	static std::string resolveShaderIncludes(const std::string& source, const std::filesystem::path& shaderPath)
	{
		static const std::regex includePattern(R"(#include\s+\"([^\"]+)\")");
		std::string resolved = source;
		std::smatch match;

		while (std::regex_search(resolved, match, includePattern))
		{
			const auto includePath = shaderPath.parent_path() / match[1].str();
			const auto includeContents = readFileContents(includePath.string());
			resolved.replace(match.position(0), match.length(0), includeContents);
		}

		return resolved;
	}

	static std::string readShaderSource(const std::string& filename)
	{
		const std::filesystem::path shaderPath(filename);
		const auto source = readFileContents(filename);
		return resolveShaderIncludes(source, shaderPath);
	}

	std::vector<VulkanRenderGraphRenderpassResources*> RenderGraph::create(
		const std::vector<hl::RenderpassInfo>& renderpassInfo, 
		VulkanDevice& device,
		uint32_t width,
		uint32_t height,
		const std::vector<VkImageView>& swapChainImageViews,
		VkFormat swapChainFormat,
		ResourceManager& /*resourceManager*/)
	{
		std::vector<VulkanRenderGraphRenderpassResources*> renderpasses;

		const auto lastName = renderpassInfo.back().name;

		for (const auto& ri : renderpassInfo)
		{
			ZoneScoped;
			ZoneNameF("Create renderpass %s", ri.name.c_str());
			auto isLastRenderpass = lastName == ri.name;

			auto imageCount = isLastRenderpass
				? (uint32_t)swapChainImageViews.size()
				: MAX_FRAMES_IN_FLIGHT;

			auto r = new VulkanRenderGraphRenderpassResources(
				ri.name,
				device, 
				imageCount);

			renderpasses.push_back(r);

			const bool useMultiSampling = passUsesMultiSampling(ri);

			{
				// images/outputs

				createImages(
					device,
					r,
					ri,
					width,
					height,
					imageCount,
					isLastRenderpass);

				std::vector<VkFormat> colorFormats;
				VkFormat depthFormat = VK_FORMAT_UNDEFINED;
				for (const auto& attachment : r->getAttachments())
				{
					if (attachment.type == ResourceType::Color)
					{
						colorFormats.push_back(isLastRenderpass ? swapChainFormat : attachment.format);
					}
					else if (attachment.type == ResourceType::Depth)
					{
						if (depthFormat != VK_FORMAT_UNDEFINED)
						{
							throw std::runtime_error("Can only have 1 depth attachment per pass");
						}
						depthFormat = attachment.format;
					}
				}

				r->setRenderingState(
					colorFormats,
					depthFormat,
					useMultiSampling ? device.msaaSamples() : VK_SAMPLE_COUNT_1_BIT,
					isLastRenderpass);

				VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

				//	Descriptor pool
				{
					uint32_t totalSets = 0;

					for (const auto& pg : ri.pipelineGroups)
					{
						for (const auto& p : pg)
						{
							totalSets += static_cast<uint32_t>(p.descriptorSets.size());
						}
					}

					if (totalSets > 0)
					{
						std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts;
						bool poolUpdateAfterBind = false;

						for (const auto& pg : ri.pipelineGroups)
						{
							for (const auto& p : pg)
							{
								for (const auto& ds : p.descriptorSets)
								{
									for (const auto& b : ds.bindings)
									{
										descriptorTypeCounts[extractDescriptorType(b.type)] += b.count;
										poolUpdateAfterBind = poolUpdateAfterBind || b.updateAfterBind;
									}
								}
							}
						}

						std::vector<VkDescriptorPoolSize> poolSizes;

						for (const auto& [type, count] : descriptorTypeCounts)
						{
							if (count > 0)
							{
								poolSizes.push_back({ type, count * imageCount });
							}
						}

						VkDescriptorPoolCreateInfo poolCreateInfo{};
						poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
						if (poolUpdateAfterBind)
						{
							poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
						}
						poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
						poolCreateInfo.pPoolSizes = poolSizes.data();
						poolCreateInfo.maxSets = static_cast<uint32_t>(imageCount) * totalSets;

						CHECK_VK_RESULT(vkCreateDescriptorPool(device.handle(), &poolCreateInfo, nullptr, &descriptorPool));

						device.setDebugName(
							reinterpret_cast<uint64_t>(descriptorPool),
							VK_OBJECT_TYPE_DESCRIPTOR_POOL,
							(r->Name + "_DescriptorPool").c_str());

						r->addDescriptorPool(descriptorPool);
					}
				}

				//	Graphics pipelines
				{
					uint32_t pipelineGroupIndex = 0;

					for (const auto& pg : ri.pipelineGroups)
					{
						r->startPipelineGroup();

						if (!pg.empty())
						{
							const auto& firstPipeline = pg.front();
							for (const auto& p : pg)
							{
								if (!descriptorSetLayoutsCompatible(firstPipeline.descriptorSets, p.descriptorSets))
								{
									throw std::runtime_error(
										"Pipeline group in pass '" + r->Name
										+ "' has incompatible descriptor layouts between '"
										+ firstPipeline.name + "' and '" + p.name + "'");
								}
							}
						}

						std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
						std::vector<VkDescriptorBindingFlags> layoutBindingFlags;
						bool layoutUpdateAfterBind = false;

						if (!pg.empty())
						{
							for (const auto& dsi : pg.front().descriptorSets)
							{
								for (const auto& b : dsi.bindings)
								{
									layoutBindings.emplace_back(
										VkDescriptorSetLayoutBinding
										{
											.binding = b.binding,
											.descriptorType = extractDescriptorType(b.type),
											.descriptorCount = b.count,
											.stageFlags = extractStage(b.stage),
											.pImmutableSamplers = nullptr
										});
									const auto flags = descriptorBindingVkFlags(b);
									layoutBindingFlags.push_back(flags);
									layoutUpdateAfterBind = layoutUpdateAfterBind
										|| ((flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) != 0);
								}
							}
						}

						VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

						if (!layoutBindings.empty())
						{
							ZoneScopedN("Create Descriptor Set Layout");
							VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
							bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
							bindingFlagsInfo.bindingCount = static_cast<uint32_t>(layoutBindingFlags.size());
							bindingFlagsInfo.pBindingFlags = layoutBindingFlags.data();

							VkDescriptorSetLayoutCreateInfo layoutInfo{};
							layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
							layoutInfo.pNext = &bindingFlagsInfo;
							if (layoutUpdateAfterBind)
							{
								layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
							}
							layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
							layoutInfo.pBindings = layoutBindings.data();

							CHECK_VK_RESULT(vkCreateDescriptorSetLayout(device.handle(), &layoutInfo, nullptr, &descriptorSetLayout));

							device.setDebugName(
								reinterpret_cast<uint64_t>(descriptorSetLayout),
								VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
								(r->Name + "_pg" + std::to_string(pipelineGroupIndex) + "_DescriptorSetLayout").c_str()
							);

							r->addPipelineGroupDescriptorSetLayout(descriptorSetLayout);
						}

						for (const auto& p : pg)
						{
							ZoneScoped;
							ZoneNameF("Create pipeline %s", p.name.c_str());

							auto& pipeline = r->addPipeline(p.name);

							pipeline.addViewportInfo(p.viewport.mode, p.viewport.width, p.viewport.height);

							VkPipelineLayout pipelineLayout;

							{
								ZoneScopedN("Create Pipeline Layout");
								// TODO: Not complete
								VkPushConstantRange pushConstantRange{};
								pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
								pushConstantRange.offset = 0;
								pushConstantRange.size = p.pushConstantSize;
								// todo: compare this size to physicalDeviceProperties.limits.maxPushConstantsSize

								VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
								pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
								if (descriptorSetLayout == VK_NULL_HANDLE)
								{
									pipelineLayoutInfo.setLayoutCount = 0;
									pipelineLayoutInfo.pSetLayouts = nullptr;
								}
								else
								{
									pipelineLayoutInfo.setLayoutCount = 1;
									pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
								}

								if (p.pushConstantSize > 0)
								{
									pipelineLayoutInfo.pushConstantRangeCount = 1;
									pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
								}

								CHECK_VK_RESULT(vkCreatePipelineLayout(device.handle(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

								device.setDebugName(
									reinterpret_cast<uint64_t>(pipelineLayout),
									VK_OBJECT_TYPE_PIPELINE_LAYOUT,
									(r->Name + "_" + p.name + "_PipelineLayout").c_str()
								);

								pipeline.addPipelineLayout(pipelineLayout);
							}

							{
								VkShaderModule vertexShaderModule;
								VkShaderModule fragmentShaderModule;

								{
									ZoneScopedN("Create Shader Modules");

									auto vertexShaderContents = readShaderSource(p.shaderVert);
									auto fragmentShaderContents = readShaderSource(p.shaderFrag);

									const auto& vertexSource = VulkanGraphicsPipeline::readParseCompileShader(
										vertexShaderContents,
										true
									);
									const auto& fragmentSource = VulkanGraphicsPipeline::readParseCompileShader(
										fragmentShaderContents,
										false
									);

									VkShaderModuleCreateInfo vertexCreateInfo{};
									vertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
									vertexCreateInfo.codeSize = vertexSource.size() * sizeof(uint32_t);
									vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexSource.data());

									CHECK_VK_RESULT(vkCreateShaderModule(device.handle(), &vertexCreateInfo, nullptr, &vertexShaderModule));

									VkShaderModuleCreateInfo fragmentCreateInfo{};
									fragmentCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
									fragmentCreateInfo.codeSize = fragmentSource.size() * sizeof(uint32_t);
									fragmentCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentSource.data());

									CHECK_VK_RESULT(vkCreateShaderModule(device.handle(), &fragmentCreateInfo, nullptr, &fragmentShaderModule));
								}

								{
									ZoneScopedN("Create Graphics Pipeline");
									VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
									vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
									vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
									vertShaderStageInfo.module = vertexShaderModule;
									vertShaderStageInfo.pName = "main";

									VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
									fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
									fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
									fragShaderStageInfo.module = fragmentShaderModule;
									fragShaderStageInfo.pName = "main";

									VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

									VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
									vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
									vertexInputInfo.vertexBindingDescriptionCount = 0;
									vertexInputInfo.vertexAttributeDescriptionCount = 0;

									std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
									vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
									vertexInputInfo.vertexBindingDescriptionCount = 0;
									vertexInputInfo.vertexAttributeDescriptionCount = 0;
									vertexInputInfo.pVertexBindingDescriptions = nullptr;
									vertexInputInfo.pVertexAttributeDescriptions = nullptr;

									VkVertexInputBindingDescription bindingDescription{};
									if (p.vertexInputInfo.has_value())
									{
										bindingDescription.binding = 0;
										bindingDescription.stride = p.vertexInputInfo.value().stride;
										bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

										for (auto& a : p.vertexInputInfo.value().attributes)
										{
											attributeDescriptions.emplace_back(VkVertexInputAttributeDescription
												{
													.location = a.location,
													.binding = 0,
													.format = extractVertexAttributeFormat(a.format),
													.offset = a.offset
												});
										}

										vertexInputInfo.vertexBindingDescriptionCount = 1;
										vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
										vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
										vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
									}

									VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
									inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
									inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
									inputAssembly.primitiveRestartEnable = VK_FALSE;

									VkPipelineViewportStateCreateInfo viewportState{};
									viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
									viewportState.viewportCount = 1;
									viewportState.scissorCount = 1;

									VkPipelineRasterizationStateCreateInfo rasterizer{};
									rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
									rasterizer.depthClampEnable = VK_FALSE;
									rasterizer.rasterizerDiscardEnable = VK_FALSE;
									rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
									rasterizer.lineWidth = 1.0f;
									rasterizer.cullMode = p.rasterState.cullMode,
									rasterizer.frontFace = p.rasterState.frontFace;
									rasterizer.depthBiasEnable = VK_FALSE;

									VkPipelineMultisampleStateCreateInfo multisampling{};
									multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
									multisampling.sampleShadingEnable = VK_TRUE;
									multisampling.rasterizationSamples = useMultiSampling
										? device.msaaSamples()
										: VK_SAMPLE_COUNT_1_BIT;
									multisampling.minSampleShading = .2f;

									VkPipelineDepthStencilStateCreateInfo depthStencil{};
									depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
									depthStencil.depthTestEnable = p.depthState.testEnable ? VK_TRUE : VK_FALSE;
									depthStencil.depthWriteEnable = p.depthState.writeEnable ? VK_TRUE : VK_FALSE;
									depthStencil.depthCompareOp = p.depthState.compareOp;
									depthStencil.depthBoundsTestEnable = VK_FALSE;
									depthStencil.stencilTestEnable = VK_FALSE;

									VkPipelineColorBlendAttachmentState colorBlendAttachment{};
									colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

									if (p.enableBlending)
									{
										colorBlendAttachment.blendEnable = VK_TRUE;
										colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
										colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
										colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
										colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
										colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
										colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
									}
									else
									{
										colorBlendAttachment.blendEnable = VK_FALSE;
									}

									VkPipelineColorBlendStateCreateInfo colorBlending{};
									colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
									colorBlending.logicOpEnable = VK_FALSE;
									colorBlending.logicOp = VK_LOGIC_OP_COPY;
									colorBlending.attachmentCount = 1;
									colorBlending.pAttachments = &colorBlendAttachment;
									colorBlending.blendConstants[0] = 0.0f;
									colorBlending.blendConstants[1] = 0.0f;
									colorBlending.blendConstants[2] = 0.0f;
									colorBlending.blendConstants[3] = 0.0f;

									std::vector<VkDynamicState> dynamicStates = {
										VK_DYNAMIC_STATE_VIEWPORT,
										VK_DYNAMIC_STATE_SCISSOR
									};
									VkPipelineDynamicStateCreateInfo dynamicState{};
									dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
									dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
									dynamicState.pDynamicStates = dynamicStates.data();

									VkGraphicsPipelineCreateInfo pipelineInfo{};
									pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
									pipelineInfo.stageCount = 2;
									pipelineInfo.pStages = shaderStages;
									pipelineInfo.pVertexInputState = &vertexInputInfo;
									pipelineInfo.pInputAssemblyState = &inputAssembly;
									pipelineInfo.pViewportState = &viewportState;
									pipelineInfo.pRasterizationState = &rasterizer;
									pipelineInfo.pMultisampleState = &multisampling;
									pipelineInfo.pDepthStencilState = &depthStencil;
									pipelineInfo.pColorBlendState = &colorBlending;
									pipelineInfo.pDynamicState = &dynamicState;
									pipelineInfo.layout = pipelineLayout;
									pipelineInfo.renderPass = VK_NULL_HANDLE;
									pipelineInfo.subpass = 0;
									pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

									VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
									pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
									pipelineRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(r->getColorFormats().size());
									pipelineRenderingInfo.pColorAttachmentFormats = r->getColorFormats().empty()
										? nullptr
										: r->getColorFormats().data();
									pipelineRenderingInfo.depthAttachmentFormat = r->getDepthFormat();
									pipelineInfo.pNext = &pipelineRenderingInfo;

									VkPipeline pl = VK_NULL_HANDLE;

									CHECK_VK_RESULT(vkCreateGraphicsPipelines(device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pl));

									device.setDebugName(
										reinterpret_cast<uint64_t>(pl),
										VK_OBJECT_TYPE_PIPELINE,
										(r->Name + "_" + p.name + "_GraphicsPipeline").c_str());

									pipeline.addPipeline(pl);
								}

								vkDestroyShaderModule(device.handle(), vertexShaderModule, nullptr);
								vkDestroyShaderModule(device.handle(), fragmentShaderModule, nullptr);
							}

							// Descriptor sets
							{
								if (descriptorSetLayout != VK_NULL_HANDLE)
								{
									std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
									VkDescriptorSetAllocateInfo allocInfo{};
									allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
									allocInfo.descriptorPool = descriptorPool;
									allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
									allocInfo.pSetLayouts = layouts.data();

									auto descriptorSets = std::vector<VkDescriptorSet>(MAX_FRAMES_IN_FLIGHT);

									CHECK_VK_RESULT(vkAllocateDescriptorSets(device.handle(), &allocInfo, descriptorSets.data()));

									for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
									{
										device.setDebugName(
											reinterpret_cast<uint64_t>(descriptorSets[i]),
											VK_OBJECT_TYPE_DESCRIPTOR_SET,
											(r->Name + "_" + p.name + "_DescriptorSet_" + std::to_string(i)).c_str());
									}

									pipeline.addDescriptorSets(descriptorSets);
								}
							}
						}

						r->endPipelineGroup();
						++pipelineGroupIndex;
					}
				}
			}
		}

		return renderpasses;
	}

	void RenderGraph::destroy(std::vector<VulkanRenderGraphRenderpassResources*>& generatedRenderpassResources)
	{
		for (auto& r : generatedRenderpassResources)
		{
			r->destroy();
		}
	}

	void RenderGraph::createImages(
		VulkanDevice& device,
		VulkanRenderGraphRenderpassResources* resources,
		const RenderpassInfo& info,
		uint32_t width,
		uint32_t height,
		uint32_t imageCount,
		bool isLastRenderpass)
	{
		std::vector<VkClearValue> clearValues;
		resources->setExtent({.width = width, .height = height});
		const bool useMultiSampling = passUsesMultiSampling(info);

		for (const auto& res : info.outputs)
		{
			auto& attachment = resources->addAttachment(res.name);
			attachment.type = res.type;
			attachment.format = extractFormat(res.format);

			if (res.type == ResourceType::Color)
			{
				if (res.clear.has_value())
				{
					clearValues.emplace_back(res.clear.value());
				}
				else
				{
					clearValues.emplace_back(VkClearValue
						{
							.color = { {0.0f, 0.0f, 0.0f, 1.0f} }
						});
				}

				if (useMultiSampling)
				{
					if (res.clear.has_value())
					{
						clearValues.emplace_back(res.clear.value());
					}
					else
					{
						clearValues.emplace_back(VkClearValue
							{
								.color = { {0.0f, 0.0f, 0.0f, 1.0f} }
							});
					}
				}
			}
			else if (res.type == ResourceType::Depth)
			{
				if (res.clear.has_value())
				{
					clearValues.emplace_back(res.clear.value());
				}
				else
				{
					clearValues.emplace_back(VkClearValue
						{
							.depthStencil = { 1.0f, 0 }
						});
				}
			}
			else
			{
				throw std::runtime_error("TODO NOT IMPLEMENTED - CLEAR VALUES GENERATION");
			}

			VkImageUsageFlags usage = res.type == ResourceType::Color
				? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
				: VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			if (useMultiSampling)
			{
				usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			}
			else
			{
				usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			}

			for (uint32_t i = 0; i < imageCount; ++i)
			{
				if (res.type == ResourceType::Color)
				{
					auto image = new VulkanImage(device);
					attachment.images.push_back(image);

					image->create(
						width,
						height,
						1, // TODO
						useMultiSampling
							? device.msaaSamples()
							: VK_SAMPLE_COUNT_1_BIT,
						attachment.format,
						VK_IMAGE_TILING_OPTIMAL,
						usage,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						1);

					image->createImageView(
						attachment.format,
						VK_IMAGE_ASPECT_COLOR_BIT,
						1);

					device.setDebugName(
						reinterpret_cast<uint64_t>(image->_image),
						VK_OBJECT_TYPE_IMAGE,
						(info.name + "_" + res.name + std::string("_Image_") + std::to_string(0)).c_str());

					device.setDebugName(
						reinterpret_cast<uint64_t>(image->_imageMemory),
						VK_OBJECT_TYPE_DEVICE_MEMORY,
						(info.name + "_" + res.name + std::string("_ImageMemory_") + std::to_string(i)).c_str());

					device.setDebugName(
						reinterpret_cast<uint64_t>(image->_imageView),
						VK_OBJECT_TYPE_IMAGE_VIEW,
						(info.name + "_" + res.name + std::string("_ImageView_") + std::to_string(i)).c_str());

					// Dont create resolve images for the last one.  Needs to do swap chain magic
					if (useMultiSampling && !isLastRenderpass)
					{
						auto resolveImage = new VulkanImage(device);
						attachment.resolveImages.push_back(resolveImage);

						resolveImage->create(
							width,
							height,
							1,
							VK_SAMPLE_COUNT_1_BIT,
							attachment.format,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							1);

						resolveImage->createImageView(
							attachment.format,
							VK_IMAGE_ASPECT_COLOR_BIT,
							1);

						device.setDebugName(
							reinterpret_cast<uint64_t>(resolveImage->_image),
							VK_OBJECT_TYPE_IMAGE,
							(info.name + "_" + res.name + std::string("_ResolveImage_") + std::to_string(i)).c_str());

						device.setDebugName(
							reinterpret_cast<uint64_t>(resolveImage->_imageMemory),
							VK_OBJECT_TYPE_DEVICE_MEMORY,
							(info.name + "_" + res.name + std::string("_ResolveImageMemory_") + std::to_string(i)).c_str());

						device.setDebugName(
							reinterpret_cast<uint64_t>(resolveImage->_imageView),
							VK_OBJECT_TYPE_IMAGE_VIEW,
							(info.name + "_" + res.name + std::string("_ResolveImageView_") + std::to_string(i)).c_str());

					}
				}
				else if (res.type == ResourceType::Depth)
				{
					auto image = new VulkanImage(device);
					attachment.images.push_back(image);

					image->create(
						width,
						height,
						1,
						useMultiSampling
							? device.msaaSamples()
							: VK_SAMPLE_COUNT_1_BIT,
						attachment.format,
						VK_IMAGE_TILING_OPTIMAL,
						usage,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						1);

					image->createImageView(
						attachment.format,
						VK_IMAGE_ASPECT_DEPTH_BIT,
						1);

					device.setDebugName(
						reinterpret_cast<uint64_t>(image->_image),
						VK_OBJECT_TYPE_IMAGE,
						(info.name + "_" + res.name + std::string("_Depth_") + std::to_string(i)).c_str());

					device.setDebugName(
						reinterpret_cast<uint64_t>(image->_imageMemory),
						VK_OBJECT_TYPE_DEVICE_MEMORY,
						(info.name + "_" + res.name + std::string("_DepthMemory_") + std::to_string(i)).c_str());

					device.setDebugName(
						reinterpret_cast<uint64_t>(image->_imageView),
						VK_OBJECT_TYPE_IMAGE_VIEW,
						(info.name + "_" + res.name + std::string("_DepthView_") + std::to_string(i)).c_str());

				}
				else
				{
					throw std::runtime_error("TODO NOT IMPLEMENTED");
				}
			}
		}

		resources->setClearValues(clearValues);
	}

	VkFormat RenderGraph::extractFormat(const std::string& formatString)
	{
		if (formatString == "VK_FORMAT_R8G8B8A8_UNORM")
		{
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
		else if (formatString == "VK_FORMAT_D32_SFLOAT")
		{
			return VK_FORMAT_D32_SFLOAT;
		}
		else if (formatString == "VK_FORMAT_B8G8R8A8_UNORM")
		{
			return VK_FORMAT_B8G8R8A8_UNORM;
		}
		else if (formatString == "VK_FORMAT_B8G8R8A8_SRGB")
		{
			return VK_FORMAT_B8G8R8A8_SRGB;
		}
		else
		{
			throw std::runtime_error("Unhandled image format.");
		}
	}

	VkDescriptorType RenderGraph::extractDescriptorType(const std::string& descriptorTypeString)
	{
		if (descriptorTypeString == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER")
		{
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		else if (descriptorTypeString == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC")
		{
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		}
		else if (descriptorTypeString == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER")
		{
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
		else if (descriptorTypeString == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER")
		{
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		else if (descriptorTypeString == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC")
		{
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		}
		else
		{
			throw std::runtime_error("Invalid descriptor type");
		}
	}

	uint32_t RenderGraph::extractStage(const std::string& stage)
	{
		uint32_t flags = 0;

		if (stage.contains("VERTEX"))
		{
			flags |= VK_SHADER_STAGE_VERTEX_BIT;
		}

		if (stage.contains("FRAGMENT"))
		{
			flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		return flags;
	}

	VkFormat RenderGraph::extractVertexAttributeFormat(VertexAttributeFormat format)
	{
		switch (format)
		{
		case VertexAttributeFormat::Float:
			return VK_FORMAT_R32_SFLOAT;
		case VertexAttributeFormat::Vec2:
			return VK_FORMAT_R32G32_SFLOAT;
		case VertexAttributeFormat::Vec3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexAttributeFormat::Vec4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			throw std::runtime_error("Invalid vertex attribute format");
		}
	}

	std::unordered_map<std::string, Node> RenderGraph::generateDAG(const std::vector<hl::RenderpassInfo>& renderpassInfo)
	{
		std::unordered_map<std::string, std::string> whoWritesWhatOutput;

		for (const auto& r : renderpassInfo)
		{
			for (const auto& o : r.outputs)
			{
				whoWritesWhatOutput.insert({ o.name, r.name });
			}
		}

		std::unordered_map<std::string, Node> nodes;

		for (const auto& rpi : renderpassInfo)
		{
			nodes.insert(
				{
					rpi.name,
					Node
					{
						.name = rpi.name
					}
				});
		}

		for (const auto& rpi : renderpassInfo)
		{
			auto& node = nodes[rpi.name];

			for (const auto& i : rpi.inputs)
			{
				if (whoWritesWhatOutput.find(i) == whoWritesWhatOutput.end())
				{
					throw std::runtime_error("No producer for specified input");
				}
				
				auto& inputProducer = nodes[whoWritesWhatOutput[i]];

				node.prev.push_back(inputProducer.name);
				inputProducer.next.push_back(node.name);
			}
		}

		auto&& computeLevel = [&](auto&& self, Node& _n) -> uint32_t
			{
				if (_n.state == Node::Visit::Visiting)
				{
					throw std::runtime_error("Cycle detected in renderpass DAG: " + _n.name);
				}

				_n.state = Node::Visit::Visiting;

				if (_n.prev.empty())
				{
					_n.layer = 0;
				}
				else
				{
					uint32_t maxParent = 0;
					for (auto& parent : _n.prev)
					{
						maxParent = std::max(maxParent, self(self, nodes[parent]));
					}

					_n.layer = maxParent + 1;
				}

				_n.state = Node::Visit::Done;

				return _n.layer;
			};

		for (auto& [nodeName, node] : nodes)
		{
			if (node.layer == std::numeric_limits<uint32_t>::max())
			{
				computeLevel(computeLevel, node);
			}
		}

		uint32_t maxLayer = 0;
		for (auto& [_, node] : nodes)
		{
			if (node.layer > maxLayer)
			{
				maxLayer = node.layer;
			}
		}

		size_t topLayerCount = 0;
		for (auto& [_, node] : nodes)
		{
			if (node.layer == maxLayer)
			{
				topLayerCount++;
			}
		}

		if (topLayerCount > 1)
		{
			throw std::runtime_error(
				"RenderGraph is invalid: multiple nodes exist at the top layer");
		}

		if (topLayerCount == 0)
		{
			throw std::runtime_error(
				"RenderGraph is invalid: No nodes exist.");
		}

		return nodes;
	}

	std::vector<GraphImageBarrierEdge> RenderGraph::generateImageBarrierEdges(
		const std::vector<hl::RenderpassInfo>& renderpassInfo)
	{
		std::vector<GraphImageBarrierEdge> edges;
		if (renderpassInfo.empty())
		{
			return edges;
		}

		const auto& lastPassName = renderpassInfo.back().name;

		for (const auto& pass : renderpassInfo)
		{
			for (const auto& output : pass.outputs)
			{
				edges.push_back(GraphImageBarrierEdge
					{
						.passName = pass.name,
						.resourceName = output.name,
						.kind = output.type == ResourceType::Depth
							? GraphImageBarrierKind::UndefinedToDepthAttachment
							: GraphImageBarrierKind::UndefinedToColorAttachment
					});
			}

			for (const auto& input : pass.inputs)
			{
				edges.push_back(GraphImageBarrierEdge
					{
						.passName = pass.name,
						.resourceName = input,
						.kind = GraphImageBarrierKind::ColorAttachmentToSampled
					});
			}
		}

		for (const auto& output : renderpassInfo.back().outputs)
		{
			if (output.type == ResourceType::Color)
			{
				edges.push_back(GraphImageBarrierEdge
					{
						.passName = lastPassName,
						.resourceName = output.name,
						.kind = GraphImageBarrierKind::ColorAttachmentToPresent
					});
			}
		}

		return edges;
	}

}