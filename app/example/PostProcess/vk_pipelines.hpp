#pragma once

#include "vk_types.hpp"

namespace vkutil
{

    bool load_shader_module(const char* filePath,
        VkDevice device,
        VkShaderModule* outShaderModule);

}