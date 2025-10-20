#pragma once

// TODO: TO CONST ARRAY NOT VECTOR
#include <vector>
#include <vulkan/vulkan.h>

#ifdef HELSINKI_DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace hl
{
	class RendererConfiguration
	{
	public:
		RendererConfiguration();
	};
}