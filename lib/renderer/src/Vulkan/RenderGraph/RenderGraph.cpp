#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

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

	std::vector<VulkanRenderGraphRenderpassResources*> RenderGraph::create(
		RenderResourcesSystem& /*renderResourcesSystem*/,
		const std::vector<hl::RenderpassInfo>& renderpassInfo, 
		VulkanDevice& device,
		uint32_t width,
		uint32_t height,
		const std::vector<VkImageView>& swapChainImageViews)
	{
		std::vector<VulkanRenderGraphRenderpassResources*> renderpasses;

		const auto lastName = renderpassInfo.back().name;

		for (const auto& ri : renderpassInfo)
		{
			auto isLastRenderpass = lastName == ri.name;

			auto imageCount = isLastRenderpass
				? (uint32_t)swapChainImageViews.size()
				: MAX_FRAMES_IN_FLIGHT;

			auto r = new VulkanRenderGraphRenderpassResources(
				ri.name,
				device, 
				imageCount);

			renderpasses.push_back(r);

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

				// Renderpass
				{
					std::vector<VkAttachmentDescription> allAttachments;

					std::vector<VkAttachmentDescription> colorAttachments;
					std::vector<VkAttachmentReference> colorReferences;

					std::vector<VkAttachmentDescription> colorResolveAttachments;
					std::vector<VkAttachmentReference> colorResolveReferences;

					std::vector<VkAttachmentDescription> depthAttachments;
					std::vector<VkAttachmentReference> depthReferences;

					uint32_t referenceLayout = 0;

					for (const auto& ra : r->getAttachments())
					{
						VkAttachmentDescription description{};
						VkAttachmentReference reference{};

						description.format = ra.format;
						description.samples = ri.useMultiSampling
							? device._msaaSamples
							: VK_SAMPLE_COUNT_1_BIT;
						description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
						description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

						reference.attachment = referenceLayout;
						referenceLayout += 1;

						if (ra.type == ResourceType::Color)
						{
							description.finalLayout = ri.useMultiSampling
								? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
								: (isLastRenderpass
									? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
									: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

							std::cout << "[DEBUG] RenderPass '" << ri.name << "' attachment '" << ra.name
								<< "' finalLayout: " << description.finalLayout << " (as uint)\n";

									reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

									colorAttachments.push_back(description);
									colorReferences.push_back(reference);

									allAttachments.push_back(description);

									if (ri.useMultiSampling)
									{
										VkAttachmentDescription resolveDescription{};
										VkAttachmentReference resolveReference{};

										resolveDescription.format = ra.format;
										resolveDescription.samples = VK_SAMPLE_COUNT_1_BIT;
										resolveDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
										resolveDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
										resolveDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
										resolveDescription.finalLayout = isLastRenderpass
											? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
											: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

										resolveReference.attachment = referenceLayout;
										resolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
										referenceLayout += 1;

										colorResolveAttachments.push_back(resolveDescription);
										colorResolveReferences.push_back(resolveReference);

										std::cout << "[DEBUG] RenderPass '" << ri.name << "' resolve attachment '" << ra.name
											<< "' finalLayout: " << resolveDescription.finalLayout << " (as uint)\n";

										allAttachments.push_back(resolveDescription);
									}
						}
						else if (ra.type == ResourceType::Depth)
						{
							description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
							description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
							description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

							reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

							depthAttachments.push_back(description);
							depthReferences.push_back(reference);

							allAttachments.push_back(description);
						}
						else
						{
							throw std::runtime_error("NOT GOOD");
						}
					}

					if (depthReferences.size() > 1)
					{
						throw std::runtime_error("Can only have 1 depth attachment per subpass");
					}

					VkSubpassDescription subpass{};
					subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
					subpass.pColorAttachments = colorReferences.data();
					subpass.pDepthStencilAttachment = depthReferences.data();
					if (ri.useMultiSampling)
					{
						subpass.pResolveAttachments = colorResolveReferences.data();
					}
					else
					{
						subpass.pResolveAttachments = nullptr;
					}

					// TODO: Optimisation possibility
					VkSubpassDependency dependency{};
					dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
					dependency.dstSubpass = 0;
					dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

					// TODO: This will change if subpasses added
					VkRenderPassCreateInfo renderPassInfo{};
					renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
					renderPassInfo.attachmentCount = static_cast<uint32_t>(allAttachments.size());
					renderPassInfo.pAttachments = allAttachments.data();
					renderPassInfo.subpassCount = 1;
					renderPassInfo.pSubpasses = &subpass;
					renderPassInfo.dependencyCount = 1;
					renderPassInfo.pDependencies = &dependency;

					VkRenderPass vkRenderpass{ VK_NULL_HANDLE };

					if (vkCreateRenderPass(device._device, &renderPassInfo, nullptr, &vkRenderpass) != VK_SUCCESS)
					{
						throw std::runtime_error("failed to create render pass!");
					}

					device.setDebugName(reinterpret_cast<uint64_t>(vkRenderpass), VK_OBJECT_TYPE_RENDER_PASS, r->Name.c_str());

					r->addRenderpass(vkRenderpass);
				}

				// Framebuffers
				{
					createFrameBuffers(
						device,
						r, 
						ri,
						width,
						height,
						swapChainImageViews,
						imageCount,
						isLastRenderpass);
				}

				VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

				//	Descriptor pool
				{
					// TODO: Need better than this maybe can calculate it from renderpass description
					std::vector<VkDescriptorPoolSize> poolSizes
					{
					   { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 * imageCount },
					   { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 * imageCount },
					};

					VkDescriptorPoolCreateInfo poolCreateInfo{};
					poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
					poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
					poolCreateInfo.pPoolSizes = poolSizes.data();
					poolCreateInfo.maxSets = static_cast<uint32_t>(imageCount);

					if (vkCreateDescriptorPool(device._device, &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
					{
						throw std::runtime_error("failed to create descriptor pool!");
					}

					device.setDebugName(
						reinterpret_cast<uint64_t>(descriptorPool),
						VK_OBJECT_TYPE_DESCRIPTOR_POOL,
						(r->Name + "_DescriptorPool").c_str());

					r->addDescriptorPool(descriptorPool);
				}

				//	Graphics pipelines
				{
					for (const auto& g : ri.pipelines)
					{
						// create descritpor set layouts

						auto& pipeline = r->addPipeline(g.name);

						std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

						for (const auto& dsi : g.descriptorSets)
						{
							for (const auto& b : dsi.bindings)
							{
								layoutBindings.emplace_back(
									VkDescriptorSetLayoutBinding
									{
										.binding = b.binding,
										.descriptorType = extractDescriptorType(b.type),
										.descriptorCount = 1,
										.stageFlags = (uint32_t)(b.stage == "VERTEX" 
											? VK_SHADER_STAGE_VERTEX_BIT
											: (b.stage == "FRAGMENT"
												? VK_SHADER_STAGE_FRAGMENT_BIT
												: throw std::runtime_error("Invalid shader stage"))),
										.pImmutableSamplers = nullptr
									});
							}						
						}

						VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

						if (!layoutBindings.empty())
						{
							VkDescriptorSetLayoutCreateInfo layoutInfo{};
							layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
							layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
							layoutInfo.pBindings = layoutBindings.data();

							if (vkCreateDescriptorSetLayout(device._device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
							{
								throw std::runtime_error("failed to create descriptor set layout!");
							}

							device.setDebugName(
								reinterpret_cast<uint64_t>(descriptorSetLayout),
								VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
								(r->Name + "_" + g.name + std::string("_DescriptorSetLayout")).c_str()
							);

							pipeline.addDescriptorSetLayout(descriptorSetLayout);
						}

						VkPipelineLayout pipelineLayout;

						{
							// TODO: Not complete
							VkPushConstantRange pushConstantRange{};
							pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
							pushConstantRange.offset = 0;
							pushConstantRange.size = 64;// TODO: Provide this??? sizeof(glm::mat4);
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
							pipelineLayoutInfo.pushConstantRangeCount = 1;
							pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;


							if (vkCreatePipelineLayout(device._device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
							{
								throw std::runtime_error("failed to create pipeline layout!");
							}

							device.setDebugName(
								reinterpret_cast<uint64_t>(pipelineLayout),
								VK_OBJECT_TYPE_PIPELINE_LAYOUT,
								(r->Name + "_" + g.name + "_PipelineLayout").c_str()
							);

							pipeline.addPipelineLayout(pipelineLayout);
						}

						{
							const auto& vertexSource = VulkanGraphicsPipeline::readParseCompileShader(
								readFileContents(g.shaderVert),
								true
							);
							const auto& fragmentSource = VulkanGraphicsPipeline::readParseCompileShader(
								readFileContents(g.shaderFrag),
								false
							);


							VkShaderModule vertexShaderModule;
							VkShaderModule fragmentShaderModule;

							{
								VkShaderModuleCreateInfo vertexCreateInfo{};
								vertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
								vertexCreateInfo.codeSize = vertexSource.size() * sizeof(uint32_t);
								vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexSource.data());

								if (vkCreateShaderModule(device._device, &vertexCreateInfo, nullptr, &vertexShaderModule) != VK_SUCCESS)
								{
									throw std::runtime_error("failed to create vertex shader module!");
								}

								VkShaderModuleCreateInfo fragmentCreateInfo{};
								fragmentCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
								fragmentCreateInfo.codeSize = fragmentSource.size() * sizeof(uint32_t);
								fragmentCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentSource.data());

								if (vkCreateShaderModule(device._device, &fragmentCreateInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS)
								{
									throw std::runtime_error("failed to create fragment shader module!");
								}
							}

							{

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

								if (g.vertexInputInfo.has_value())
								{
									VkVertexInputBindingDescription bindingDescription{};
									bindingDescription.binding = 0;
									bindingDescription.stride = g.vertexInputInfo.value().stride;
									bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

									for (auto& a : g.vertexInputInfo.value().attributes)
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
								if (g.enableCulling)
								{
									rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
								}
								else
								{
									rasterizer.cullMode = VK_CULL_MODE_NONE;
								}
								rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
								rasterizer.depthBiasEnable = VK_FALSE;

								VkPipelineMultisampleStateCreateInfo multisampling{};
								multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
								multisampling.sampleShadingEnable = VK_TRUE;
								multisampling.rasterizationSamples = ri.useMultiSampling
									? device._msaaSamples
									: VK_SAMPLE_COUNT_1_BIT;
								multisampling.minSampleShading = .2f;

								VkPipelineDepthStencilStateCreateInfo depthStencil{};
								depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
								depthStencil.depthTestEnable = VK_FALSE;
								depthStencil.depthWriteEnable = VK_FALSE;

								if (g.enableDepthTest)
								{
									depthStencil.depthTestEnable = VK_TRUE;
									depthStencil.depthWriteEnable = VK_TRUE;
									depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
									depthStencil.depthBoundsTestEnable = VK_FALSE;
									depthStencil.stencilTestEnable = VK_FALSE;
								}

								VkPipelineColorBlendAttachmentState colorBlendAttachment{};
								colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
								
								if (g.enableBlending)
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
								pipelineInfo.renderPass = r->getRenderPass();
								pipelineInfo.subpass = 0;
								pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

								VkPipeline p = VK_NULL_HANDLE;

								if (vkCreateGraphicsPipelines(device._device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p) != VK_SUCCESS)
								{
									throw std::runtime_error("failed to create graphics pipeline!");
								}

								device.setDebugName(
									reinterpret_cast<uint64_t>(p),
									VK_OBJECT_TYPE_PIPELINE,
									(r->Name + "_" + g.name + "_GraphicsPipeline").c_str());

								pipeline.addPipeline(p);
							}

							vkDestroyShaderModule(device._device, vertexShaderModule, nullptr);
							vkDestroyShaderModule(device._device, fragmentShaderModule, nullptr);
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
								if (vkAllocateDescriptorSets(device._device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
								{
									throw std::runtime_error("Failed to allocate descriptor sets!");
								}

								for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
								{
									device.setDebugName(
										reinterpret_cast<uint64_t>(descriptorSets[i]),
										VK_OBJECT_TYPE_DESCRIPTOR_SET,
										(r->Name + "_" + g.name + "_DescriptorSet_" + std::to_string(i)).c_str());
								}

								pipeline.addDescriptorSets(descriptorSets);
							}
						}
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

		for (const auto& res : info.outputs)
		{
			// TODO: Foreach N where N is MAX_FRAMES_IN_FLIGHT or swapchain image count
			auto& attachment = resources->addAttachment(res.name);
			attachment.type = res.type;
			attachment.format = extractFormat(res.format);

			if (res.type == ResourceType::Color)
			{
				clearValues.emplace_back(VkClearValue
					{
						.color = { {0.0f, 0.0f, 0.0f, 1.0f} }
					});

				if (info.useMultiSampling)
				{
					clearValues.emplace_back(VkClearValue
						{
							.color = { {0.0f, 0.0f, 0.0f, 1.0f} }
						});
				}
			}
			else if (res.type == ResourceType::Depth)
			{
				clearValues.emplace_back(VkClearValue
					{
						.depthStencil = { 1.0f, 0 }
					});
			}
			else
			{
				throw std::runtime_error("TODO NOT IMPLEMENTED - CLEAR VALUES GENERATION");
			}

			VkImageUsageFlags usage = res.type == ResourceType::Color
				? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
				: VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			if (info.useMultiSampling)
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
						info.useMultiSampling
							? device._msaaSamples
							: VK_SAMPLE_COUNT_1_BIT,
						attachment.format,
						VK_IMAGE_TILING_OPTIMAL,
						usage,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
					if (info.useMultiSampling && !isLastRenderpass)
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
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
						info.useMultiSampling
							? device._msaaSamples
							: VK_SAMPLE_COUNT_1_BIT,
						attachment.format,
						VK_IMAGE_TILING_OPTIMAL,
						usage,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
	void RenderGraph::createFrameBuffers(
		VulkanDevice& device,
		VulkanRenderGraphRenderpassResources* resources, 
		const RenderpassInfo& info,
		uint32_t width,
		uint32_t height,
		const std::vector<VkImageView>& swapChainImageViews,
		uint32_t imageCount,
		bool isLastRenderpass)
	{
		for (uint32_t i = 0; i < imageCount; ++i)
		{
			VkFramebuffer f = VK_NULL_HANDLE;
			std::vector<VkImageView> attachments;

			if (isLastRenderpass)
			{
				auto foundColorAttachment = false;

				if (info.useMultiSampling)
				{
					for (auto& a : resources->getAttachments())
					{
						if (a.type == ResourceType::Color)
						{
							if (foundColorAttachment)
							{
								throw std::runtime_error("Cannot have multiple color attachments in renderpass that writes to the swapchain.");
							}
							else
							{
								foundColorAttachment = true;
							}
						}

						// these size to i comparisons, probably should be size vs image count
						if (a.images.size() == imageCount)
						{
							attachments.push_back(a.images[i]->_imageView);
						}

						if (a.resolveImages.size() == imageCount ||
							a.type == ResourceType::Color)
						{
							if (a.type == ResourceType::Color)
							{
								// TODO: Assert that swapchain image format matches a.format
								attachments.push_back(swapChainImageViews[i]);
							}
							else
							{
								attachments.push_back(a.resolveImages[i]->_imageView);
							}
						}
					}

				}
				else
				{
					for (auto& a : resources->getAttachments())
					{
						if (a.type == ResourceType::Color)
						{
							if (foundColorAttachment)
							{
								throw std::runtime_error("Cannot have multiple color attachments in renderpass that writes to the swapchain.");
							}
							else
							{
								foundColorAttachment = true;
							}
						}

						// these size to i comparisons, probably should be size vs image count
						if (a.images.size() == imageCount)
						{
							if (a.type == ResourceType::Color)
							{
								// TODO: Assert that swapchain image format matches a.format
								attachments.push_back(swapChainImageViews[i]);
							}
							else
							{
								attachments.push_back(a.images[i]->_imageView);
							}
						}

						if (a.resolveImages.size() > 0)
						{
							throw std::runtime_error("Should not have resolve images when not using multi sampling");
						}
					}
				}
			}
			else
			{
				if (info.useMultiSampling)
				{
					for (auto& a : resources->getAttachments())
					{
						// these size to i comparisons, probably should be size vs image count
						if (a.images.size() == imageCount)
						{
							attachments.push_back(a.images[i]->_imageView);
						}

						if (a.resolveImages.size() == imageCount)
						{
							attachments.push_back(a.resolveImages[i]->_imageView);
						}
					}
				}
				else
				{
					for (auto& a : resources->getAttachments())
					{
						// these size to i comparisons, probably should be size vs image count
						if (a.images.size() == imageCount)
						{
							attachments.push_back(a.images[i]->_imageView);
						}

						if (a.resolveImages.size() > 0)
						{
							throw std::runtime_error("Should not have resolve images when not using multi sampling");
						}
					}
				}
			}

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = resources->getRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device._device, &framebufferInfo, nullptr, &f) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}

			std::cout << "[DEBUG] Renderpass " << info.name << " - framebuffer " << i << " attachments:\n";
			for (size_t ji = 0; ji < attachments.size(); ji++)
			{
				std::cout << "  Attachment " << ji << ": " << reinterpret_cast<uint64_t>(attachments[ji]) << "\n";
			}

			device.setDebugName(
				reinterpret_cast<uint64_t>(f),
				VK_OBJECT_TYPE_FRAMEBUFFER,
				(resources->Name + std::string("_Framebuffer_") + std::to_string(i)).c_str()
			);

			resources->addFramebuffer(f);
		}
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
		else if (descriptorTypeString == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER")
		{
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
		else
		{
			throw std::runtime_error("Invalid descriptor type");
		}
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

}