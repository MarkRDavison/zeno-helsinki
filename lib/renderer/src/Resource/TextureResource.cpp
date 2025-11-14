#include <helsinki/Renderer/Resource/TextureResource.hpp>

namespace hl
{
	TextureResource::TextureResource(
		const std::string& id, 
		VulkanDevice& device
	) : 
		Resource(id),
		_device(device),
		_texture(_device)
	{
	}
}