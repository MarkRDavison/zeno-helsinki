#pragma once

#include <optional>
#include <vector>

namespace hl
{
    struct VulkanQueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily; 
        std::optional<uint32_t> computeFamily;
        std::vector<uint32_t>   transferFamilies; // store multiple if you want N


        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
}