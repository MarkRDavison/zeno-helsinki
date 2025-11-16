#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/System/HelsinkiTracy.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

namespace hl
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        VulkanDevice& device
    ) :
        _device(device),
        _pipelineLayout(device)
    {

    }

    static std::string readFile(const std::string& filename)
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

    static VkShaderModule createShaderModule(VulkanDevice& device, const std::vector<uint32_t>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        CHECK_VK_RESULT(vkCreateShaderModule(device._device, &createInfo, nullptr, &shaderModule));

        return shaderModule;
    }

    std::vector<uint32_t> VulkanGraphicsPipeline::readParseCompileShader(
        const std::string& shaderSource, 
        bool vertex)
    {
        // TODO: Compute etc
        ZoneScoped;
        
        ZoneNameF("readParseCompileShader: %s", vertex ? "Vertex" : "Fragment");
        EShLanguage stage = vertex ? EShLanguage::EShLangVertex : EShLanguage::EShLangFragment;

        glslang::TShader shader(stage);
        const char* sources[] = { shaderSource.c_str() };
        shader.setStrings(sources, 1);
        shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 450);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
        shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

        TBuiltInResource DefaultTBuiltInResource = {};
        {
            // Maximum counts
            DefaultTBuiltInResource.maxLights = 32;
            DefaultTBuiltInResource.maxClipPlanes = 6;
            DefaultTBuiltInResource.maxTextureUnits = 32;
            DefaultTBuiltInResource.maxTextureCoords = 32;
            DefaultTBuiltInResource.maxVertexAttribs = 64;
            DefaultTBuiltInResource.maxVertexUniformComponents = 4096;
            DefaultTBuiltInResource.maxVaryingFloats = 64;
            DefaultTBuiltInResource.maxVertexTextureImageUnits = 32;
            DefaultTBuiltInResource.maxCombinedTextureImageUnits = 80;
            DefaultTBuiltInResource.maxTextureImageUnits = 32;
            DefaultTBuiltInResource.maxFragmentUniformComponents = 4096;
            DefaultTBuiltInResource.maxDrawBuffers = 32;

            // Compute shader limits (if needed)
            DefaultTBuiltInResource.maxComputeWorkGroupCountX = 65535;
            DefaultTBuiltInResource.maxComputeWorkGroupCountY = 65535;
            DefaultTBuiltInResource.maxComputeWorkGroupCountZ = 65535;
            DefaultTBuiltInResource.maxComputeWorkGroupSizeX = 1024;
            DefaultTBuiltInResource.maxComputeWorkGroupSizeY = 1024;
            DefaultTBuiltInResource.maxComputeWorkGroupSizeZ = 64;
            DefaultTBuiltInResource.maxComputeUniformComponents = 1024;
            DefaultTBuiltInResource.maxComputeTextureImageUnits = 16;
            DefaultTBuiltInResource.maxComputeImageUniforms = 8;
            DefaultTBuiltInResource.maxComputeAtomicCounters = 8;
            DefaultTBuiltInResource.maxComputeAtomicCounterBuffers = 1;

            // General limits
            DefaultTBuiltInResource.maxLights = 32;
            DefaultTBuiltInResource.maxClipPlanes = 6;
            DefaultTBuiltInResource.maxTextureUnits = 32;
            DefaultTBuiltInResource.maxVertexAttribs = 64;

            // Boolean flags
            DefaultTBuiltInResource.limits.nonInductiveForLoops = 1;
            DefaultTBuiltInResource.limits.whileLoops = 1;
            DefaultTBuiltInResource.limits.doWhileLoops = 1;
            DefaultTBuiltInResource.limits.generalUniformIndexing = 1;
            DefaultTBuiltInResource.limits.generalAttributeMatrixVectorIndexing = 1;
            DefaultTBuiltInResource.limits.generalVaryingIndexing = 1;
            DefaultTBuiltInResource.limits.generalSamplerIndexing = 1;
            DefaultTBuiltInResource.limits.generalVariableIndexing = 1;
            DefaultTBuiltInResource.limits.generalConstantMatrixVectorIndexing = 1;
        }

        shader.parse(&DefaultTBuiltInResource, 0, false, EShMsgDefault);

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
        {
            std::cerr << "GLSL link failed: " << program.getInfoLog() << std::endl;
            return {};
        }

        std::vector<uint32_t> spirv;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

        return spirv;
    }

	void VulkanGraphicsPipeline::create(
		const std::string& vertexPath, 
		const std::string& fragmentPath,
        VulkanRenderpass& renderpass,
		VulkanDescriptorSetLayout& descriptorSetLayout,
        bool multiSampling)
	{
        _pipelineLayout.create(descriptorSetLayout);

		auto vertShaderCode = readFile(vertexPath);
		auto fragShaderCode = readFile(fragmentPath);

        auto vertexSpirv = readParseCompileShader(vertShaderCode, true);
        auto fragmentSpirv = readParseCompileShader(fragShaderCode, false);

		VkShaderModule vertShaderModule = createShaderModule(_device, vertexSpirv);
		VkShaderModule fragShaderModule = createShaderModule(_device, fragmentSpirv);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.rasterizationSamples = multiSampling
            ? _device._msaaSamples
            : VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

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
        pipelineInfo.layout = _pipelineLayout._pipelineLayout;
        pipelineInfo.renderPass = renderpass._renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        CHECK_VK_RESULT(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

        vkDestroyShaderModule(_device._device, fragShaderModule, nullptr);
        vkDestroyShaderModule(_device._device, vertShaderModule, nullptr);
	}

    void VulkanGraphicsPipeline::createPostProcess(
        const std::string& vertexPath,
        const std::string& fragmentPath,
        VulkanRenderpass& renderpass,
        VulkanDescriptorSetLayout& descriptorSetLayout,
        bool multiSampling)
    {
        _pipelineLayout.create(descriptorSetLayout);

        auto vertShaderCode = readFile(vertexPath);
        auto fragShaderCode = readFile(fragmentPath);

        auto vertexSpirv = readParseCompileShader(vertShaderCode, true);
        auto fragmentSpirv = readParseCompileShader(fragShaderCode, false);

        VkShaderModule vertShaderModule = createShaderModule(_device, vertexSpirv);
        VkShaderModule fragShaderModule = createShaderModule(_device, fragmentSpirv);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // --- NO vertex input for fullscreen triangle ---
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        // input assembly: triangle list (we will issue vkCmdDraw(cmd, 3, 1, 0, 0))
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // viewport/scissor will be dynamic
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // rasterizer: no culling (safe for full-screen triangle)
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE; // IMPORTANT for fullscreen triangle
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // no MSAA for post-process (single-sample)
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = multiSampling ? VK_TRUE : VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;

        // disable depth/stencil for post-process
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // color blend (simple overwrite)
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;


        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

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

        // dynamic states: viewport + scissor
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // assemble pipeline
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
        pipelineInfo.layout = _pipelineLayout._pipelineLayout;
        pipelineInfo.renderPass = renderpass._renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        CHECK_VK_RESULT(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

        vkDestroyShaderModule(_device._device, fragShaderModule, nullptr);
        vkDestroyShaderModule(_device._device, vertShaderModule, nullptr);
    }

    void VulkanGraphicsPipeline::createUi(
        const std::string& vertexPath,
        const std::string& fragmentPath,
        VulkanRenderpass& renderpass,
        VulkanDescriptorSetLayout& descriptorSetLayout,
        bool multiSampling,
        bool part2)
    {
        _pipelineLayout.create(descriptorSetLayout);

        auto vertShaderCode = readFile(vertexPath);
        auto fragShaderCode = readFile(fragmentPath);

        auto vertexSpirv = readParseCompileShader(vertShaderCode, true);
        auto fragmentSpirv = readParseCompileShader(fragShaderCode, false);

        VkShaderModule vertShaderModule = createShaderModule(_device, vertexSpirv);
        VkShaderModule fragShaderModule = createShaderModule(_device, fragmentSpirv);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // --- NO vertex input for fullscreen triangle ---
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        // input assembly: triangle list (we will issue vkCmdDraw(cmd, 3, 1, 0, 0))
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // viewport/scissor will be dynamic
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // rasterizer: no culling (safe for full-screen triangle)
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE; // IMPORTANT for fullscreen triangle
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // no MSAA for ui (single-sample)
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = multiSampling ? VK_TRUE : VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;

        // disable depth/stencil for ui
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // color blend (simple overwrite)
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        if (part2)
        {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
        else
        {
            colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

        // dynamic states: viewport + scissor
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // assemble pipeline
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
        pipelineInfo.layout = _pipelineLayout._pipelineLayout;
        pipelineInfo.renderPass = renderpass._renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        CHECK_VK_RESULT(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

        vkDestroyShaderModule(_device._device, fragShaderModule, nullptr);
        vkDestroyShaderModule(_device._device, vertShaderModule, nullptr);
    }

	void VulkanGraphicsPipeline::destroy()
	{
		vkDestroyPipeline(_device._device, _graphicsPipeline, nullptr);
		_pipelineLayout.destroy();
	}
}