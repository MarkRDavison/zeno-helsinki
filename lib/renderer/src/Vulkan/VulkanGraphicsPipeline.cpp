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

        EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);
        if (!shader.parse(&DefaultTBuiltInResource, 0, false, messages))
        {
            std::cerr
                << "GLSL link failed: " << std::endl
                << shader.getInfoDebugLog() << std::endl
                << shader.getInfoLog() << std::endl;
            return {};
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages))
        {
            std::cerr 
                << "GLSL link failed: " << std::endl 
                << program.getInfoLog() << std::endl
                << shader.getInfoLog() << std::endl;
            return {};
        }

        std::vector<uint32_t> spirv;
        glslang::SpvOptions options{}; 
        
        // TODO: only for debug
        options.disableOptimizer = true;    // better for debugging
        options.validate = true;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &options);

        return spirv;
    }

	void VulkanGraphicsPipeline::destroy()
	{
		vkDestroyPipeline(_device._device, _graphicsPipeline, nullptr);
		_pipelineLayout.destroy();
	}
}