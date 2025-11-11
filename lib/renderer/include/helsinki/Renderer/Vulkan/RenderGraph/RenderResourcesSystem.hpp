#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <helsinki/Renderer/RendererConfiguration.hpp>

struct hash_pair
{
	template <class T1, class T2>
	size_t operator()(const std::pair<T1, T2>& p) const
	{
		size_t hash1 = std::hash<T1>{}(p.first);
		size_t hash2 = std::hash<T2>{}(p.second);

		return hash1
			^ (hash2 + 0x9e3779b9 + (hash1 << 6)
				+ (hash1 >> 2));
	}
};


namespace hl
{
	class VulkanImage;
	class VulkanTexture;
	class VulkanUniformBuffer;

	enum class RenderResourceType
	{
		Texture,
		UniformBuffer,
		RenderpassOutputImage,

		Count
	};

	struct RenderResource
	{
		std::string name;
		RenderResourceType type{ RenderResourceType::Count };

		VulkanTexture& getTexture();
		VulkanUniformBuffer& getUniformBuffer(uint32_t frame);
		std::pair<VkImageView, VkSampler> getOffscreenImageWithSampler(uint32_t frame);
		
	private:
		friend class RenderResourcesSystem;
		VulkanTexture* _texture{ nullptr };
		std::vector<VulkanUniformBuffer*> _uniformBuffers;
		std::vector<VulkanImage*> _offscreenImages;
		VkSampler _offscreenImageSampler{ VK_NULL_HANDLE };
	};

	class RenderResourcesSystem
	{
	public:
		RenderResourcesSystem();

		void addTexture(const std::string& name, VulkanTexture* texture);
		void addUniformBuffers(const std::string& name, std::vector<VulkanUniformBuffer*> uniformBuffers);
		void registerOffscreenImage(const std::string& name, std::vector<VulkanImage*> images, VkSampler sampler);

		VulkanTexture& getTexture(const std::string& name) const;
		VulkanUniformBuffer& getUniformBuffer(const std::string& name, uint32_t frame) const;
		std::pair<VkImageView, VkSampler> getOffscreenImageWithSampler(const std::string& name, uint32_t frame);

		std::pair<VkImageView, VkSampler> getOffscreenImageOrTexture(const std::string& name, uint32_t frame);

	private:
		std::unordered_map<
			std::pair<std::string, RenderResourceType>, 
			RenderResource*, 
			hash_pair> _resources;
	};

}