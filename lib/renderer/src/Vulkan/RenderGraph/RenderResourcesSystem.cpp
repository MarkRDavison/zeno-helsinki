#include <helsinki/Renderer/Vulkan/RenderGraph/RenderResourcesSystem.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <stdexcept>

namespace hl
{

	VulkanTexture& RenderResource::getTexture()
	{
		if (_texture == nullptr)
		{
			throw std::runtime_error("Cannot cast to texture");
		}

		return *_texture;
	}

	VulkanUniformBuffer& RenderResource::getUniformBuffer(uint32_t frame)
	{
		if (_uniformBuffers.size() <= frame)
		{
			throw std::runtime_error("Cannot retrieve uniform buffer");
		}

		return *_uniformBuffers[frame];
	}

	std::pair<VkImageView, VkSampler> RenderResource::getOffscreenImageWithSampler(uint32_t frame)
	{
		if (_offscreenImageSampler == VK_NULL_HANDLE ||
			_offscreenImages.size() <= frame)
		{
			throw std::runtime_error("Cannot retrieve offscreen image and sampler");
		}

		return { _offscreenImages[frame]->_imageView, _offscreenImageSampler };
	}

	RenderResourcesSystem::RenderResourcesSystem()
	{

	}

	void RenderResourcesSystem::addTexture(const std::string& name, VulkanTexture* texture)
	{
		if (_resources.find({ name, RenderResourceType::Texture }) != _resources.end())
		{
			throw std::runtime_error("Cannot add duplicate resource");
		}

		auto res = new RenderResource();
		res->name = name;
		res->type = RenderResourceType::Texture;
		res->_texture = texture;

		_resources.insert({ {name, RenderResourceType::Texture }, res });
	}
	void RenderResourcesSystem::addUniformBuffers(const std::string& name, std::vector<VulkanUniformBuffer*> uniformBuffers)
	{
		if (_resources.find({ name, RenderResourceType::UniformBuffer }) != _resources.end())
		{
			throw std::runtime_error("Cannot add duplicate resource");
		}

		auto res = new RenderResource();
		res->name = name;
		res->type = RenderResourceType::UniformBuffer;
		res->_uniformBuffers = std::vector<VulkanUniformBuffer*>(uniformBuffers);

		_resources.insert({ {name, RenderResourceType::UniformBuffer }, res });
	}
	void RenderResourcesSystem::registerOffscreenImage(const std::string& name, std::vector<VulkanImage*> images, VkSampler sampler)
	{
		if (_resources.find({ name, RenderResourceType::RenderpassOutputImage }) != _resources.end())
		{
			throw std::runtime_error("Cannot add duplicate resource");
		}

		auto res = new RenderResource();
		res->name = name;
		res->type = RenderResourceType::RenderpassOutputImage;
		res->_offscreenImages = std::vector<VulkanImage*>(images);
		res->_offscreenImageSampler = sampler;

		_resources.insert({ {name, RenderResourceType::RenderpassOutputImage }, res });
	}

	VulkanTexture& RenderResourcesSystem::getTexture(const std::string& name) const
	{
		auto iter = _resources.find({ name, RenderResourceType::Texture });

		if (iter == _resources.end())
		{
			throw std::runtime_error("Cannot find texture given name");
		}

		return (*iter).second->getTexture();
	}
	VulkanUniformBuffer& RenderResourcesSystem::getUniformBuffer(const std::string& name, uint32_t frame) const
	{
		auto iter = _resources.find({ name, RenderResourceType::UniformBuffer });

		if (iter == _resources.end())
		{
			throw std::runtime_error("Cannot find uniform buffer given name");
		}

		return (*iter).second->getUniformBuffer(frame);
	}
	std::pair<VkImageView, VkSampler> RenderResourcesSystem::getOffscreenImageWithSampler(const std::string& name, uint32_t frame)
	{
		auto iter = _resources.find({ name, RenderResourceType::RenderpassOutputImage });

		if (iter == _resources.end())
		{
			throw std::runtime_error("Cannot find offscreen image and sampler given name");
		}

		return (*iter).second->getOffscreenImageWithSampler(frame);
	}

	std::pair<VkImageView, VkSampler> RenderResourcesSystem::getOffscreenImageOrTexture(const std::string& name, uint32_t frame)
	{
		auto iter = _resources.find({ name, RenderResourceType::RenderpassOutputImage });

		if (iter != _resources.end())
		{
			return (*iter).second->getOffscreenImageWithSampler(frame);
		}

		auto texture = getTexture(name);

		return { texture._image._imageView, texture._sampler };
	}

}