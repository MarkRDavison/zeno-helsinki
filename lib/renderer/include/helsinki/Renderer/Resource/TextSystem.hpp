#pragma once

#include <string>
#include <unordered_map>
#include <queue>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/Renderer/Resource/Text.hpp>

namespace hl
{

	class TextSystem : NonCopyable
	{
	public:
		TextSystem(
			VulkanDevice& device,
			VulkanCommandPool& transferPool,
			ResourceManager& resourceManager);

		int registerText(const std::string& text, const std::string& font, unsigned size);
		int registerText(int id, const std::string& text, const std::string& font, unsigned size);
		
		void destroy();

		const Text& getText(int id) const;
		glm::vec4 getTextSize(int id) const;

		std::unordered_map<std::string, uint32_t> bindFontsDescriptor(
			FontType fontType, 
			VkDescriptorSet descriptorSet) const;

		void processDeferredTextDestruction(int count = -1);

	private:
		void generateText(int id, const std::string& text, const std::string& font, unsigned size);

	private:
		VulkanDevice& _device;
		VulkanCommandPool& _transferPool;
		ResourceManager& _resourceManager;
		int _nextId{ 0 };
		std::unordered_map<int, Text*> _textInfo;
		std::queue<int> _textToDestroy;
	};

}